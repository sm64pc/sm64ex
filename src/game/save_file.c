#include <ultra64.h>
#include "sm64.h"
#include "game_init.h"
#include "main.h"
#include "engine/math_util.h"
#include "area.h"
#include "level_update.h"
#include "save_file.h"
#include "sound_init.h"
#include "level_table.h"
#include "course_table.h"
#include "thread6.h"
#include "macros.h"
#include "pc/ini.h"
#include "moon/config/saves/saves.h"

#define MENU_DATA_MAGIC 0x4849
#define SAVE_FILE_MAGIC 0x4441

extern struct SaveBuffer gSaveBuffer;

struct WarpCheckpoint gWarpCheckpoint;

s8 gMainMenuDataModified;
s8 gSaveFileModified;

u8 gLastCompletedCourseNum = COURSE_NONE;
u8 gLastCompletedStarNum = 0;
s8 sUnusedGotGlobalCoinHiScore = 0;
u8 gGotFileCoinHiScore = 0;
u8 gCurrCourseStarFlags = 0;

u8 gSpecialTripleJump = 0;

#define STUB_LEVEL(_0, _1, courseenum, _3, _4, _5, _6, _7, _8) courseenum,
#define DEFINE_LEVEL(_0, _1, courseenum, _3, _4, _5, _6, _7, _8, _9, _10) courseenum,

s8 gLevelToCourseNumTable[] = {
    #include "levels/level_defines.h"
};
#undef STUB_LEVEL
#undef DEFINE_LEVEL

STATIC_ASSERT(ARRAY_COUNT(gLevelToCourseNumTable) == LEVEL_COUNT - 1,
              "change this array if you are adding levels");

static s32 get_coin_score_age(s32 fileIndex, s32 courseIndex) {
    return (gSaveBuffer.menuData[0].coinScoreAges[fileIndex] >> (2 * courseIndex)) & 0x3;
}

static void set_coin_score_age(s32 fileIndex, s32 courseIndex, s32 age) {
    s32 mask = 0x3 << (2 * courseIndex);

    gSaveBuffer.menuData[0].coinScoreAges[fileIndex] &= ~mask;
    gSaveBuffer.menuData[0].coinScoreAges[fileIndex] |= age << (2 * courseIndex);
}

/**
 * Mark a coin score for a save file as the newest out of all save files.
 */
static void touch_coin_score_age(s32 fileIndex, s32 courseIndex) {
    s32 i;
    u32 age;
    u32 currentAge = get_coin_score_age(fileIndex, courseIndex);

    if (currentAge != 0) {
        for (i = 0; i < NUM_SAVE_FILES; i++) {
            age = get_coin_score_age(i, courseIndex);
            if (age < currentAge) {
                set_coin_score_age(i, courseIndex, age + 1);
            }
        }

        set_coin_score_age(fileIndex, courseIndex, 0);
        gMainMenuDataModified = TRUE;
    }
}

/**
 * Mark all coin scores for a save file as new.
 */
static void touch_high_score_ages(s32 fileIndex) {
    s32 i;

    for (i = 0; i < 15; i++) {
        touch_coin_score_age(fileIndex, i);
    }
}

void save_file_do_save(s32 fileIndex) {
    if (fileIndex < 0 || fileIndex >= NUM_SAVE_FILES)
        return;

    if (gSaveFileModified) {
        // Write to text file
        writeSaveFile(fileIndex);
        gSaveFileModified = FALSE;
        gMainMenuDataModified = FALSE;
    }
}

void save_file_erase(s32 fileIndex) {
    if (fileIndex < 0 || fileIndex >= NUM_SAVE_FILES)
        return;

    touch_high_score_ages(fileIndex);
    bzero(&gSaveBuffer.files[fileIndex][0], sizeof(gSaveBuffer.files[fileIndex][0]));

    gSaveFileModified = TRUE;
    eraseSaveFile(fileIndex);

}

//! Needs to be s32 to match on -O2, despite no return value.
BAD_RETURN(s32) save_file_copy(s32 srcFileIndex, s32 destFileIndex) {
    if (srcFileIndex < 0 || srcFileIndex >= NUM_SAVE_FILES || destFileIndex < 0 || destFileIndex >= NUM_SAVE_FILES)
        return;

    touch_high_score_ages(destFileIndex);
    bcopy(&gSaveBuffer.files[srcFileIndex][0], &gSaveBuffer.files[destFileIndex][0],
          sizeof(gSaveBuffer.files[destFileIndex][0]));

    gSaveFileModified = TRUE;
    save_file_do_save(destFileIndex);
}

void save_file_load_all(void) {
    s32 file;

    gMainMenuDataModified = FALSE;
    gSaveFileModified = FALSE;

    bzero(&gSaveBuffer, sizeof(gSaveBuffer));

    for (file = 0; file < NUM_SAVE_FILES; file++) {
        readSaveFile(file);
    }
    gSaveFileModified = TRUE;
    gMainMenuDataModified = TRUE;
}

/**
 * Reload the current save file from its backup copy, which is effectively a
 * a cached copy of what has been written to EEPROM.
 * This is used after getting a game over.
 */
void save_file_reload(void) {
    // Copy save file data from backup
    bcopy(&gSaveBuffer.files[gCurrSaveFileNum - 1][1], &gSaveBuffer.files[gCurrSaveFileNum - 1][0],
          sizeof(gSaveBuffer.files[gCurrSaveFileNum - 1][0]));

    // Copy main menu data from backup
    bcopy(&gSaveBuffer.menuData[1], &gSaveBuffer.menuData[0], sizeof(gSaveBuffer.menuData[0]));

    gMainMenuDataModified = FALSE;
    gSaveFileModified = FALSE;
}

/**
 * Update the current save file after collecting a star or a key.
 * If coin score is greater than the current high score, update it.
 */
void save_file_collect_star_or_key(s16 coinScore, s16 starIndex) {
    s32 fileIndex = gCurrSaveFileNum - 1;
    s32 courseIndex = gCurrCourseNum - 1;

    s32 starFlag = 1 << starIndex;
    UNUSED s32 flags = save_file_get_flags();

    gLastCompletedCourseNum = courseIndex + 1;
    gLastCompletedStarNum = starIndex + 1;
    sUnusedGotGlobalCoinHiScore = 0;
    gGotFileCoinHiScore = 0;

    if (courseIndex >= 0 && courseIndex < COURSE_STAGES_COUNT) {
        //! Compares the coin score as a 16 bit value, but only writes the 8 bit
        // truncation. This can allow a high score to decrease.

        if (coinScore > ((u16) save_file_get_max_coin_score(courseIndex) & 0xFFFF)) {
            sUnusedGotGlobalCoinHiScore = 1;
        }

        if (coinScore > save_file_get_course_coin_score(fileIndex, courseIndex)) {
            gSaveBuffer.files[fileIndex][0].courseCoinScores[courseIndex] = coinScore;
            touch_coin_score_age(fileIndex, courseIndex);

            gGotFileCoinHiScore = 1;
            gSaveFileModified = TRUE;
        }
    }

    switch (gCurrLevelNum) {
        case LEVEL_BOWSER_1:
            if (!(save_file_get_flags() & (SAVE_FLAG_HAVE_KEY_1 | SAVE_FLAG_UNLOCKED_BASEMENT_DOOR))) {
                save_file_set_flags(SAVE_FLAG_HAVE_KEY_1);
            }
            break;

        case LEVEL_BOWSER_2:
            if (!(save_file_get_flags() & (SAVE_FLAG_HAVE_KEY_2 | SAVE_FLAG_UNLOCKED_UPSTAIRS_DOOR))) {
                save_file_set_flags(SAVE_FLAG_HAVE_KEY_2);
            }
            break;

        case LEVEL_BOWSER_3:
            break;

        default:
            if (!(save_file_get_star_flags(fileIndex, courseIndex) & starFlag)) {
                save_file_set_star_flags(fileIndex, courseIndex, starFlag);
            }
            break;
    }
}

s32 save_file_exists(s32 fileIndex) {
    return (gSaveBuffer.files[fileIndex][0].flags & SAVE_FLAG_FILE_EXISTS) != 0;
}

/**
 * Get the maximum coin score across all files for a course. The lower 16 bits
 * of the returned value are the score, and the upper 16 bits are the file number
 * of the save file with this score.
 */
u32 save_file_get_max_coin_score(s32 courseIndex) {
    s32 fileIndex;
    s32 maxCoinScore = -1;
    s32 maxScoreAge = -1;
    s32 maxScoreFileNum = 0;

    for (fileIndex = 0; fileIndex < NUM_SAVE_FILES; fileIndex++) {
        if (save_file_get_star_flags(fileIndex, courseIndex) != 0) {
            s32 coinScore = save_file_get_course_coin_score(fileIndex, courseIndex);
            s32 scoreAge = get_coin_score_age(fileIndex, courseIndex);

            if (coinScore > maxCoinScore || (coinScore == maxCoinScore && scoreAge > maxScoreAge)) {
                maxCoinScore = coinScore;
                maxScoreAge = scoreAge;
                maxScoreFileNum = fileIndex + 1;
            }
        }
    }
    return (maxScoreFileNum << 16) + max(maxCoinScore, 0);
}

s32 save_file_get_course_star_count(s32 fileIndex, s32 courseIndex) {
    s32 i;
    s32 count = 0;
    u8 flag = 1;
    u8 starFlags = save_file_get_star_flags(fileIndex, courseIndex);

    for (i = 0; i < 7; i++, flag <<= 1) {
        if (starFlags & flag) {
            count++;
        }
    }
    return count;
}

s32 save_file_get_total_star_count(s32 fileIndex, s32 minCourse, s32 maxCourse) {
    s32 count = 0;

    // Get standard course star count.
    for (; minCourse <= maxCourse; minCourse++) {
        count += save_file_get_course_star_count(fileIndex, minCourse);
    }

    // Add castle secret star count.
    return save_file_get_course_star_count(fileIndex, -1) + count;
}

void save_file_set_flags(u32 flags) {
    gSaveBuffer.files[gCurrSaveFileNum - 1][0].flags |= (flags | SAVE_FLAG_FILE_EXISTS);
    gSaveFileModified = TRUE;
}

void save_file_clear_flags(u32 flags) {
    gSaveBuffer.files[gCurrSaveFileNum - 1][0].flags &= ~flags;
    gSaveBuffer.files[gCurrSaveFileNum - 1][0].flags |= SAVE_FLAG_FILE_EXISTS;
    gSaveFileModified = TRUE;
}

u32 save_file_get_flags(void) {
    if (gCurrCreditsEntry != 0 || gCurrDemoInput != NULL) {
        return 0;
    }
    return gSaveBuffer.files[gCurrSaveFileNum - 1][0].flags;
}

/**
 * Return the bitset of obtained stars in the specified course.
 * If course is -1, return the bitset of obtained castle secret stars.
 */
u32 save_file_get_star_flags(s32 fileIndex, s32 courseIndex) {
    u32 starFlags;

    if (courseIndex == -1) {
        starFlags = (gSaveBuffer.files[fileIndex][0].flags >> 24) & 0x7F;
    } else {
        starFlags = gSaveBuffer.files[fileIndex][0].courseStars[courseIndex] & 0x7F;
    }

    return starFlags;
}

/**
 * Add to the bitset of obtained stars in the specified course.
 * If course is -1, add to the bitset of obtained castle secret stars.
 */
void save_file_set_star_flags(s32 fileIndex, s32 courseIndex, u32 starFlags) {
    if (courseIndex == -1) {
        gSaveBuffer.files[fileIndex][0].flags |= starFlags << 24;
    } else {
        gSaveBuffer.files[fileIndex][0].courseStars[courseIndex] |= starFlags;
    }

    gSaveBuffer.files[fileIndex][0].flags |= SAVE_FLAG_FILE_EXISTS;
    gSaveFileModified = TRUE;
}

s32 save_file_get_course_coin_score(s32 fileIndex, s32 courseIndex) {
    return gSaveBuffer.files[fileIndex][0].courseCoinScores[courseIndex];
}

/**
 * Return TRUE if the cannon is unlocked in the current course.
 */
s32 save_file_is_cannon_unlocked(void) {
    return (gSaveBuffer.files[gCurrSaveFileNum - 1][0].courseStars[gCurrCourseNum] & 0x80) != 0;
}

/**
 * Sets the cannon status to unlocked in the current course.
 */
void save_file_set_cannon_unlocked(void) {
    gSaveBuffer.files[gCurrSaveFileNum - 1][0].courseStars[gCurrCourseNum] |= 0x80;
    gSaveBuffer.files[gCurrSaveFileNum - 1][0].flags |= SAVE_FLAG_FILE_EXISTS;
    gSaveFileModified = TRUE;
}

void save_file_set_cap_pos(s16 x, s16 y, s16 z) {
    struct SaveFile *saveFile = &gSaveBuffer.files[gCurrSaveFileNum - 1][0];

    saveFile->capLevel = gCurrLevelNum;
    saveFile->capArea = gCurrAreaIndex;
    vec3s_set(saveFile->capPos, x, y, z);
    save_file_set_flags(SAVE_FLAG_CAP_ON_GROUND);
}

s32 save_file_get_cap_pos(Vec3s capPos) {
    struct SaveFile *saveFile = &gSaveBuffer.files[gCurrSaveFileNum - 1][0];
    s32 flags = save_file_get_flags();

    if (saveFile->capLevel == gCurrLevelNum && saveFile->capArea == gCurrAreaIndex
        && (flags & SAVE_FLAG_CAP_ON_GROUND)) {
        vec3s_copy(capPos, saveFile->capPos);
        return TRUE;
    }
    return FALSE;
}

void save_file_set_sound_mode(u16 mode) {
    set_sound_mode(mode);
    gSaveBuffer.menuData[0].soundMode = mode;

    gMainMenuDataModified = TRUE;
}

u16 save_file_get_sound_mode(void) {
    return gSaveBuffer.menuData[0].soundMode;
}

void save_file_move_cap_to_default_location(void) {
    if (save_file_get_flags() & SAVE_FLAG_CAP_ON_GROUND) {
        switch (gSaveBuffer.files[gCurrSaveFileNum - 1][0].capLevel) {
            case LEVEL_SSL:
                save_file_set_flags(SAVE_FLAG_CAP_ON_KLEPTO);
                break;
            case LEVEL_SL:
                save_file_set_flags(SAVE_FLAG_CAP_ON_MR_BLIZZARD);
                break;
            case LEVEL_TTM:
                save_file_set_flags(SAVE_FLAG_CAP_ON_UKIKI);
                break;
        }
        save_file_clear_flags(SAVE_FLAG_CAP_ON_GROUND);
    }
}

void disable_warp_checkpoint(void) {
    // check_warp_checkpoint() checks to see if gWarpCheckpoint.courseNum != COURSE_NONE
    gWarpCheckpoint.courseNum = COURSE_NONE;
}

/**
 * Checks the upper bit of the WarpNode->destLevel byte to see if the
 * game should set a warp checkpoint.
 */
void check_if_should_set_warp_checkpoint(struct WarpNode *warpNode) {
    if (warpNode->destLevel & 0x80) {
        // Overwrite the warp checkpoint variables.
        gWarpCheckpoint.actNum = gCurrActNum;
        gWarpCheckpoint.courseNum = gCurrCourseNum;
        gWarpCheckpoint.levelID = warpNode->destLevel & 0x7F;
        gWarpCheckpoint.areaNum = warpNode->destArea;
        gWarpCheckpoint.warpNode = warpNode->destNode;
    }
}

/**
 * Checks to see if a checkpoint is properly active or not. This will
 * also update the level, area, and destination node of the input WarpNode.
 * returns TRUE if input WarpNode was updated, and FALSE if not.
 */
s32 check_warp_checkpoint(struct WarpNode *warpNode) {
    s16 isWarpCheckpointActive = FALSE;
    s16 currCourseNum = gLevelToCourseNumTable[(warpNode->destLevel & 0x7F) - 1];

    // gSavedCourseNum is only used in this function.
    if (gWarpCheckpoint.courseNum != COURSE_NONE && gSavedCourseNum == currCourseNum
        && gWarpCheckpoint.actNum == gCurrActNum) {
        warpNode->destLevel = gWarpCheckpoint.levelID;
        warpNode->destArea = gWarpCheckpoint.areaNum;
        warpNode->destNode = gWarpCheckpoint.warpNode;
        isWarpCheckpointActive = TRUE;
    } else {
        // Disable the warp checkpoint just in case the other 2 conditions failed?
        gWarpCheckpoint.courseNum = COURSE_NONE;
    }

    return isWarpCheckpointActive;
}

u32 save_file_get_cannon_flags(s32 fileIndex, s32 courseIndex) {

    if (gSaveBuffer.files[fileIndex][0].courseStars[courseIndex+1] & 0x80){return 1;}

    return 0;
}