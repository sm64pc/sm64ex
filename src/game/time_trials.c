#include "time_trials.h"
#include "sm64.h"
#include "game_init.h"
#include "sound_init.h"
#include "geo_commands.h"
#include "gfx_dimensions.h"
#include "ingame_menu.h"
#include "level_update.h"
#include "object_helpers.h"
#include "object_list_processor.h"
#include "save_file.h"
#include "segment2.h"
#include "spawn_object.h"
#include "pc/cheats.h"
#include "pc/cliopts.h"
#include "pc/configfile.h"
#include "pc/fs/fs.h"
#include "engine/math_util.h"
#include "audio/external.h"
#include <stdio.h>

#define TIME_TRIALS_MAX_ALLOWED_TIME    17999 // 9'59"99
#define TIME_TRIALS_UNDEFINED_TIME      -1
#ifndef OMM_STARS_NON_STOP
#define OMM_STARS_NON_STOP              (0)
#endif
#ifndef OMM_ALL_STARS
#define OMM_ALL_STARS                   (0)
#endif

static bool sTimeTrialsEnabled = true;
bool time_trials_enabled() {
    return sTimeTrialsEnabled;
}

//
// Models
//

#include "time_trials_ghost_geo.inl"

static const BehaviorScript bhvTimeTrialGhost1[] = { 0x08000000, 0x09000000 };
static const BehaviorScript bhvTimeTrialGhost2[] = { 0x08000000, 0x09000000 };

enum {
    GHOST_MARIO_DEFAULT = 0,
    GHOST_MARIO_RED_COINS,
    GHOST_MARIO_PSS_2,
    GHOST_MARIO_100_COINS,
    GHOST_MARIO_ALL_STARS
};
static struct GraphNode *time_trials_get_ghost_mario_graph_node(u8 color) {
    static void *sGhostMarios[5][2] = {
        { (void *) ghost_mario_blue_geo,  NULL }, // Default ghost
        { (void *) ghost_mario_red_geo,   NULL }, // Bowser Red coins ghost
        { (void *) ghost_mario_green_geo, NULL }, // PSS 2 ghost
        { (void *) ghost_mario_gold_geo,  NULL }, // 100 coins ghost
        { (void *) ghost_mario_pink_geo,  NULL }, // All Stars ghost
    };
    if (sGhostMarios[0][1] == NULL) {
        for (u8 i = 0; i != 5; ++i) {
            struct AllocOnlyPool *pool = calloc(1, 0x10000);
            pool->totalSpace   = 0x10000 - sizeof(struct AllocOnlyPool);
            pool->usedSpace    = 0;
            pool->startPtr     = (u8 *) pool + sizeof(struct AllocOnlyPool);
            pool->freePtr      = (u8 *) pool + sizeof(struct AllocOnlyPool);
            sGhostMarios[i][1] = process_geo_layout(pool, sGhostMarios[i][0]);
        }
    }
    return sGhostMarios[color][1];
}

//
// Utils
//

static const char *time_trials_get_save_filename() {
    static char buffer[256];
    snprintf(buffer, 256, "%s", SAVE_FILENAME);
    buffer[strlen(buffer) - 4] = 0; // cut extension
    return buffer;
}

static const char *time_trials_get_folder(const char *path) {
    static char buffer[256];
    snprintf(buffer, 256, "%s/%s", path, time_trials_get_save_filename());
    return buffer;
}

// For retro-compatibility
static const char *time_trials_get_old_filename(const char *path, s32 fileIndex, s32 slotIndex) {
    static char buffer[256];
    snprintf(buffer, 256, "%s/%d.%d.ttg", time_trials_get_folder(path), fileIndex, slotIndex);
    return buffer;
}

static const char *time_trials_get_new_filename(const char *path, s32 fileIndex, s32 courseIndex, s32 starIndex) {
    static const char *sCourseTags[] = {
        "bob", "wf", "jrb", "ccm", "bbh", "hmc", "lll", "ssl", "ddd", "sl", "wdw", "ttm", "thi", "ttc", "rr",
        "bitdw", "bitfs", "bits", "pss", "cotmc", "totwc", "vcutm", "wmotr", "sa"
    };
    static char buffer[256];
    snprintf(buffer, 256, "%s/%d.%s.%d.ttg", time_trials_get_folder(path), fileIndex, sCourseTags[courseIndex], starIndex);
    return buffer;
}

static const u8 *time_trials_int_to_sm64_string(s32 value, const char *format) {
    static u8 buffer[64];
    bzero(buffer, 64);
    if (!format || strlen(format) > 8) {
        return buffer;
    }
    s32 len = snprintf((char *) buffer, 64, format, value);
    for (s32 i = 0; i < len; ++i) {
        if (buffer[i] >= 'A' && buffer[i] <= 'F') {
            buffer[i] = buffer[i] - 'A';
        } else if (buffer[i] >= 'a' && buffer[i] <= 'f') {
            buffer[i] = buffer[i] - 'a';
        } else {
            buffer[i] = buffer[i] - '0';
        }
    }
    buffer[len] = DIALOG_CHAR_TERMINATOR;
    return buffer;
}

//
// Time Trials Slots
//

#define NUM_SLOTS                   sizeof(sTimeTrialSlots) / sizeof(sTimeTrialSlots[0])
#define STAR_CODE(course, star)     (u16)(((u16)(course & 0xFF) << 8) | ((u16)(star & 0xFF) << 0))

static const u16 sTimeTrialSlots[] = {

    /* Main courses */
    STAR_CODE(COURSE_BOB, 0),
    STAR_CODE(COURSE_BOB, 1),
    STAR_CODE(COURSE_BOB, 2),
    STAR_CODE(COURSE_BOB, 3),
    STAR_CODE(COURSE_BOB, 4),
    STAR_CODE(COURSE_BOB, 5),
    STAR_CODE(COURSE_BOB, 6),
    STAR_CODE(COURSE_WF, 0),
    STAR_CODE(COURSE_WF, 1),
    STAR_CODE(COURSE_WF, 2),
    STAR_CODE(COURSE_WF, 3),
    STAR_CODE(COURSE_WF, 4),
    STAR_CODE(COURSE_WF, 5),
    STAR_CODE(COURSE_WF, 6),
    STAR_CODE(COURSE_JRB, 0),
    STAR_CODE(COURSE_JRB, 1),
    STAR_CODE(COURSE_JRB, 2),
    STAR_CODE(COURSE_JRB, 3),
    STAR_CODE(COURSE_JRB, 4),
    STAR_CODE(COURSE_JRB, 5),
    STAR_CODE(COURSE_JRB, 6),
    STAR_CODE(COURSE_CCM, 0),
    STAR_CODE(COURSE_CCM, 1),
    STAR_CODE(COURSE_CCM, 2),
    STAR_CODE(COURSE_CCM, 3),
    STAR_CODE(COURSE_CCM, 4),
    STAR_CODE(COURSE_CCM, 5),
    STAR_CODE(COURSE_CCM, 6),
    STAR_CODE(COURSE_BBH, 0),
    STAR_CODE(COURSE_BBH, 1),
    STAR_CODE(COURSE_BBH, 2),
    STAR_CODE(COURSE_BBH, 3),
    STAR_CODE(COURSE_BBH, 4),
    STAR_CODE(COURSE_BBH, 5),
    STAR_CODE(COURSE_BBH, 6),
    STAR_CODE(COURSE_HMC, 0),
    STAR_CODE(COURSE_HMC, 1),
    STAR_CODE(COURSE_HMC, 2),
    STAR_CODE(COURSE_HMC, 3),
    STAR_CODE(COURSE_HMC, 4),
    STAR_CODE(COURSE_HMC, 5),
    STAR_CODE(COURSE_HMC, 6),
    STAR_CODE(COURSE_LLL, 0),
    STAR_CODE(COURSE_LLL, 1),
    STAR_CODE(COURSE_LLL, 2),
    STAR_CODE(COURSE_LLL, 3),
    STAR_CODE(COURSE_LLL, 4),
    STAR_CODE(COURSE_LLL, 5),
    STAR_CODE(COURSE_LLL, 6),
    STAR_CODE(COURSE_SSL, 0),
    STAR_CODE(COURSE_SSL, 1),
    STAR_CODE(COURSE_SSL, 2),
    STAR_CODE(COURSE_SSL, 3),
    STAR_CODE(COURSE_SSL, 4),
    STAR_CODE(COURSE_SSL, 5),
    STAR_CODE(COURSE_SSL, 6),
    STAR_CODE(COURSE_DDD, 0),
    STAR_CODE(COURSE_DDD, 1),
    STAR_CODE(COURSE_DDD, 2),
    STAR_CODE(COURSE_DDD, 3),
    STAR_CODE(COURSE_DDD, 4),
    STAR_CODE(COURSE_DDD, 5),
    STAR_CODE(COURSE_DDD, 6),
    STAR_CODE(COURSE_SL, 0),
    STAR_CODE(COURSE_SL, 1),
    STAR_CODE(COURSE_SL, 2),
    STAR_CODE(COURSE_SL, 3),
    STAR_CODE(COURSE_SL, 4),
    STAR_CODE(COURSE_SL, 5),
    STAR_CODE(COURSE_SL, 6),
    STAR_CODE(COURSE_WDW, 0),
    STAR_CODE(COURSE_WDW, 1),
    STAR_CODE(COURSE_WDW, 2),
    STAR_CODE(COURSE_WDW, 3),
    STAR_CODE(COURSE_WDW, 4),
    STAR_CODE(COURSE_WDW, 5),
    STAR_CODE(COURSE_WDW, 6),
    STAR_CODE(COURSE_TTM, 0),
    STAR_CODE(COURSE_TTM, 1),
    STAR_CODE(COURSE_TTM, 2),
    STAR_CODE(COURSE_TTM, 3),
    STAR_CODE(COURSE_TTM, 4),
    STAR_CODE(COURSE_TTM, 5),
    STAR_CODE(COURSE_TTM, 6),
    STAR_CODE(COURSE_THI, 0),
    STAR_CODE(COURSE_THI, 1),
    STAR_CODE(COURSE_THI, 2),
    STAR_CODE(COURSE_THI, 3),
    STAR_CODE(COURSE_THI, 4),
    STAR_CODE(COURSE_THI, 5),
    STAR_CODE(COURSE_THI, 6),
    STAR_CODE(COURSE_TTC, 0),
    STAR_CODE(COURSE_TTC, 1),
    STAR_CODE(COURSE_TTC, 2),
    STAR_CODE(COURSE_TTC, 3),
    STAR_CODE(COURSE_TTC, 4),
    STAR_CODE(COURSE_TTC, 5),
    STAR_CODE(COURSE_TTC, 6),
    STAR_CODE(COURSE_RR, 0),
    STAR_CODE(COURSE_RR, 1),
    STAR_CODE(COURSE_RR, 2),
    STAR_CODE(COURSE_RR, 3),
    STAR_CODE(COURSE_RR, 4),
    STAR_CODE(COURSE_RR, 5),
    STAR_CODE(COURSE_RR, 6),

    /* Bowser Courses */
    STAR_CODE(COURSE_BITDW, 0),
    STAR_CODE(COURSE_BITDW, 1),
    STAR_CODE(COURSE_BITFS, 0),
    STAR_CODE(COURSE_BITFS, 1),
    STAR_CODE(COURSE_BITS, 0),
    STAR_CODE(COURSE_BITS, 1),

    /* Secret Courses */
    STAR_CODE(COURSE_PSS, 0),
    STAR_CODE(COURSE_PSS, 1),
    STAR_CODE(COURSE_SA, 0),
    STAR_CODE(COURSE_WMOTR, 0),
    STAR_CODE(COURSE_TOTWC, 0),
    STAR_CODE(COURSE_VCUTM, 0),
    STAR_CODE(COURSE_COTMC, 0),

    /* All Stars */
    // Placed here for retro-compatibility
    STAR_CODE(COURSE_BOB, 7),
    STAR_CODE(COURSE_WF, 7),
    STAR_CODE(COURSE_JRB, 7),
    STAR_CODE(COURSE_CCM, 7),
    STAR_CODE(COURSE_BBH, 7),
    STAR_CODE(COURSE_HMC, 7),
    STAR_CODE(COURSE_LLL, 7),
    STAR_CODE(COURSE_SSL, 7),
    STAR_CODE(COURSE_DDD, 7),
    STAR_CODE(COURSE_SL, 7),
    STAR_CODE(COURSE_WDW, 7),
    STAR_CODE(COURSE_TTM, 7),
    STAR_CODE(COURSE_THI, 7),
    STAR_CODE(COURSE_TTC, 7),
    STAR_CODE(COURSE_RR, 7),
};

static s32 time_trials_get_slot_index(s32 courseNum, s32 starIndex) {
    u16 starCode = STAR_CODE(courseNum, starIndex);
    for (u32 i = 0; i != NUM_SLOTS; ++i) {
        if (sTimeTrialSlots[i] == starCode) {
            return (s32) i;
        }
    }
    return -1;
}

#undef STAR_CODE

//
// Ghost Frame data
//

struct TimeTrialGhostFrameData {
    s16 posX;
    s16 posY;
    s16 posZ;
    s8 pitch;
    s8 yaw;
    s8 roll;
    u8 scaleX;
    u8 scaleY;
    u8 scaleZ;
    u8 animIndex;
    u8 animFrame;
    u8 level;
    u8 area;
};
#define time_trials_is_end_frame(p, f) (p[f].level == 0)

//
// Data
//

enum { TT_TIMER_DISABLED, TT_TIMER_RUNNING, TT_TIMER_STOPPED };
static s8 sTimeTrialTimerState = TT_TIMER_DISABLED;
static s16 sTimeTrialTimer = 0;
static bool sTimeTrialHiScore = false;
static struct TimeTrialGhostFrameData sTimeTrialGhostDataR1[TIME_TRIALS_MAX_ALLOWED_TIME]; // Read-only, normal/all-stars ghost
static struct TimeTrialGhostFrameData sTimeTrialGhostDataR2[TIME_TRIALS_MAX_ALLOWED_TIME]; // Read-only, special ghost
static struct TimeTrialGhostFrameData sTimeTrialGhostDataW[TIME_TRIALS_MAX_ALLOWED_TIME]; // Write-only

//
// Read
//

static u16 read_u16(FILE *f) {
    u8 low, high;
    if (fread(&high, 1, 1, f)) {
        if (fread(&low, 1, 1, f)) {
            return (u16)(((u16)(high) << 8) | ((u16)(low)));
        }
    }
    return 0;
}

static u8 read_u8(FILE *f) {
    u8 byte;
    if (fread(&byte, 1, 1, f)) {
        return byte;
    }
    return 0;
}

static s16 time_trials_read_ghost_data(s32 fileIndex, s32 courseIndex, s32 starIndex, s32 slotIndex, struct TimeTrialGhostFrameData* ghostData) {
    if (ghostData != NULL) {
        bzero(ghostData, (sizeof(struct TimeTrialGhostFrameData) * TIME_TRIALS_MAX_ALLOWED_TIME));
    }

    // Valid slotIndex
    if (slotIndex == -1) {
        return 0;
    }

    // Open file
    const char *filename = time_trials_get_new_filename(fs_writepath, fileIndex, courseIndex, starIndex);
    FILE *f = fopen(filename, "rb");
    if (f == NULL) {
        filename = time_trials_get_old_filename(fs_writepath, fileIndex, slotIndex);
        f = fopen(filename, "rb");
        if (f == NULL) {
            return 0;
        }
    }

    // Read data
    s16 length = (s16) read_u16(f);
    if (ghostData != NULL) {
        for (s16 i = 0; i != length; ++i) {
            ghostData[i].posX      = (s16) read_u16(f);
            ghostData[i].posY      = (s16) read_u16(f);
            ghostData[i].posZ      = (s16) read_u16(f);
            ghostData[i].pitch     = (s8) read_u8(f);
            ghostData[i].yaw       = (s8) read_u8(f);
            ghostData[i].roll      = (s8) read_u8(f);
            ghostData[i].scaleX    = read_u8(f);
            ghostData[i].scaleY    = read_u8(f);
            ghostData[i].scaleZ    = read_u8(f);
            ghostData[i].animIndex = read_u8(f);
            ghostData[i].animFrame = read_u8(f);
            ghostData[i].level     = read_u8(f);
            ghostData[i].area      = read_u8(f);
        }
    }

    fclose(f);
    return length;
}

static s16 *time_trials_time(s32 fileIndex, s32 slotIndex) {
    static s16 sTimeTrialTimes[NUM_SAVE_FILES][NUM_SLOTS];
    static bool inited = false;
    if (!inited) {
        for (s32 i = 0; i != NUM_SAVE_FILES; ++i) {
            for (s32 courseIndex = 0; courseIndex != COURSE_MAX; ++courseIndex) {
                for (s32 starIndex = 0; starIndex != 8; ++starIndex) {
                    s32 s = time_trials_get_slot_index(courseIndex + 1, starIndex);
                    if (s != -1) {
                        s16 t = time_trials_read_ghost_data(i, courseIndex, starIndex, s, NULL);
                        if (t <= 0 || t > TIME_TRIALS_MAX_ALLOWED_TIME) {
                            t = TIME_TRIALS_UNDEFINED_TIME;
                        }
                        sTimeTrialTimes[i][s] = t;
                    }
                }
            }
        }
        inited = true;
    }
    return &sTimeTrialTimes[fileIndex][slotIndex];
}

//
// Write
//

static void write_u16(FILE *f, u16 word) {
    u8 low = (u8)(word);
    u8 high = (u8)(word >> 8);
    fwrite(&high, 1, 1, f);
    fwrite(&low, 1, 1, f);
}

static void write_u8(FILE *f, u8 byte) {
    fwrite(&byte, 1, 1, f);
}

static void time_trials_write_ghost_data(s32 fileIndex, s32 courseIndex, s32 starIndex) {

    // Make folder
    fs_sys_mkdir(time_trials_get_folder(fs_writepath));

    // Open file
    const char *filename = time_trials_get_new_filename(fs_writepath, fileIndex, courseIndex, starIndex);
    FILE *f = fopen(filename, "wb");
    if (f == NULL) {
        return;
    }

    // Write length
    s16 length = sTimeTrialTimer;
    write_u16(f, length);

    // Write data
    for (s16 i = 0; i != length; ++i) {
        write_u16(f, (u16) sTimeTrialGhostDataW[i].posX);
        write_u16(f, (u16) sTimeTrialGhostDataW[i].posY);
        write_u16(f, (u16) sTimeTrialGhostDataW[i].posZ);
        write_u8(f, (u8) sTimeTrialGhostDataW[i].pitch);
        write_u8(f, (u8) sTimeTrialGhostDataW[i].yaw);
        write_u8(f, (u8) sTimeTrialGhostDataW[i].roll);
        write_u8(f, sTimeTrialGhostDataW[i].scaleX);
        write_u8(f, sTimeTrialGhostDataW[i].scaleY);
        write_u8(f, sTimeTrialGhostDataW[i].scaleZ);
        write_u8(f, sTimeTrialGhostDataW[i].animIndex);
        write_u8(f, sTimeTrialGhostDataW[i].animFrame);
        write_u8(f, sTimeTrialGhostDataW[i].level);
        write_u8(f, sTimeTrialGhostDataW[i].area);
    }

    // Update times
    *time_trials_time(fileIndex, time_trials_get_slot_index(courseIndex + 1, starIndex)) = length;
    fclose(f);
}

//
// Get
//

static s16 time_trials_get_time(s32 fileIndex, s32 courseNum, s32 starIndex) {
    s32 slotIndex = time_trials_get_slot_index(courseNum, starIndex);
    if (slotIndex == -1) return TIME_TRIALS_UNDEFINED_TIME;
    return *time_trials_time(fileIndex, slotIndex);
}

static s32 time_trials_get_course_time(s32 fileIndex, s32 courseNum) {
    s32 courseTime = 0;
    for (s32 starIndex = 0; starIndex < 7; starIndex++) {
        s32 slotIndex = time_trials_get_slot_index(courseNum, starIndex);
        s16 t = *time_trials_time(fileIndex, slotIndex);
        if (t == TIME_TRIALS_UNDEFINED_TIME) {
            return TIME_TRIALS_UNDEFINED_TIME;
        }

        courseTime += (s32) t;
    }
    return courseTime;
}

static s32 time_trials_get_bowser_time(s32 fileIndex) {
    s32 bitdw = time_trials_get_course_time(fileIndex, COURSE_BITDW);
    if (bitdw == TIME_TRIALS_UNDEFINED_TIME) {
        return TIME_TRIALS_UNDEFINED_TIME;
    }
    s32 bitfs = time_trials_get_course_time(fileIndex, COURSE_BITFS);
    if (bitfs == TIME_TRIALS_UNDEFINED_TIME) {
        return TIME_TRIALS_UNDEFINED_TIME;
    }
    s32 bits = time_trials_get_course_time(fileIndex, COURSE_BITS);
    if (bits == TIME_TRIALS_UNDEFINED_TIME) {
        return TIME_TRIALS_UNDEFINED_TIME;
    }
    return bitdw + bitfs + bits;
}

static s32 time_trials_get_secret_time(s32 fileIndex) {
    s32 pss = time_trials_get_course_time(fileIndex, COURSE_PSS);
    if (pss == TIME_TRIALS_UNDEFINED_TIME) {
        return TIME_TRIALS_UNDEFINED_TIME;
    }
    s32 cotmc = time_trials_get_course_time(fileIndex, COURSE_COTMC);
    if (cotmc == TIME_TRIALS_UNDEFINED_TIME) {
        return TIME_TRIALS_UNDEFINED_TIME;
    }
    s32 totwc = time_trials_get_course_time(fileIndex, COURSE_TOTWC);
    if (totwc == TIME_TRIALS_UNDEFINED_TIME) {
        return TIME_TRIALS_UNDEFINED_TIME;
    }
    s32 vcutm = time_trials_get_course_time(fileIndex, COURSE_VCUTM);
    if (vcutm == TIME_TRIALS_UNDEFINED_TIME) {
        return TIME_TRIALS_UNDEFINED_TIME;
    }
    s32 wmotr = time_trials_get_course_time(fileIndex, COURSE_WMOTR);
    if (wmotr == TIME_TRIALS_UNDEFINED_TIME) {
        return TIME_TRIALS_UNDEFINED_TIME;
    }
    s32 sa = time_trials_get_course_time(fileIndex, COURSE_SA);
    if (sa == TIME_TRIALS_UNDEFINED_TIME) {
        return TIME_TRIALS_UNDEFINED_TIME;
    }
    return pss + cotmc + totwc + vcutm + wmotr + sa;
}

static s32 time_trials_get_total_time(s32 fileIndex) {
    s32 coursesTime = 0;
    for (s32 course = COURSE_MIN; course <= COURSE_STAGES_MAX; course++) {
        s32 t = time_trials_get_course_time(fileIndex, course);
        if (t == TIME_TRIALS_UNDEFINED_TIME) {
            return TIME_TRIALS_UNDEFINED_TIME;
        }
        coursesTime += t;
    }

    s32 bowserTime = time_trials_get_bowser_time(fileIndex);
    if (bowserTime == TIME_TRIALS_UNDEFINED_TIME) {
        return TIME_TRIALS_UNDEFINED_TIME;
    }

    s32 secretTime = time_trials_get_secret_time(fileIndex);
    if (secretTime == TIME_TRIALS_UNDEFINED_TIME) {
        return TIME_TRIALS_UNDEFINED_TIME;
    }

    return coursesTime + bowserTime + secretTime;
}

//
// Ghost
//

struct TimeTrialGhostAnimationData {
    struct Animation animation;
    u8 animationData[0x40000];
    u8 color;
    u16 prevValidAnimIndex;
};
static struct TimeTrialGhostAnimationData sGhostAnimData1;
static struct TimeTrialGhostAnimationData sGhostAnimData2;

static u16 time_trials_ghost_retrieve_current_animation_index(struct MarioState *m) {
    struct MarioAnimDmaRelatedThing *animDmaTable = m->animation->animDmaTable;
    void *a = m->animation->currentAnimAddr;
    for (u16 i = 0; i != animDmaTable->count; ++i) {
        void *b = animDmaTable->srcAddr + animDmaTable->anim[i].offset;
        if (a == b) {
            return i;
        }
    }
    return 0;
}

static s32 time_trials_ghost_load_animation(struct TimeTrialGhostAnimationData *animData, struct MarioState *m, u16 animIndex) {
    struct MarioAnimDmaRelatedThing *animDmaTable = m->animation->animDmaTable;
    if (animIndex >= animDmaTable->count) {
        return FALSE;
    }

    // Raw data
    u8 *addr = animDmaTable->srcAddr + animDmaTable->anim[animIndex].offset;
    u32 size = animDmaTable->anim[animIndex].size;        
    bcopy(addr, animData->animationData, size);

    // Seek index and values pointers
    struct Animation *a = (struct Animation *) animData->animationData;
    const u16 *indexStart = (const u16 *) ((uintptr_t) (animData->animationData) + (uintptr_t) (a->index));
    const s16 *valueStart = (const s16 *) ((uintptr_t) (animData->animationData) + (uintptr_t) (a->values));

    // Fill ghost animation data
    bcopy(animData->animationData, &animData->animation, sizeof(struct Animation));
    animData->animation.index = indexStart;
    animData->animation.values = valueStart;
    return TRUE;
}

static void time_trials_ghost_update_animation(struct TimeTrialGhostAnimationData *animData, struct Object *obj, struct MarioState *m, u16 animIndex, u16 animFrame) {
    if (obj->header.gfx.unk38.curAnim == NULL) {
        animData->prevValidAnimIndex = 0xFFFF;
    }

    // Load & set animation
    if (animIndex != animData->prevValidAnimIndex && time_trials_ghost_load_animation(animData, m, animIndex)) {
        obj->header.gfx.unk38.curAnim = &animData->animation;
        obj->header.gfx.unk38.animAccel = 0;
        obj->header.gfx.unk38.animYTrans = m->unkB0;
        animData->prevValidAnimIndex = animIndex;
    }

    // Set frame
    if (obj->header.gfx.unk38.curAnim != NULL) {
        obj->header.gfx.unk38.animFrame = MIN(animFrame, obj->header.gfx.unk38.curAnim->unk08 - 1);
    }
}

static struct Object *obj_get_time_trials_ghost(const BehaviorScript *behavior) {
    struct ObjectNode *listHead = &gObjectLists[OBJ_LIST_DEFAULT];
    struct Object *next = (struct Object *) listHead->next;
    while (next != (struct Object *) listHead) {
        if (next->behavior == behavior && next->activeFlags != 0) {
            return next;
        }
        next = (struct Object *) next->header.next;
    }
    return NULL;
}

static void time_trials_update_ghost(struct TimeTrialGhostFrameData *ghostData, struct TimeTrialGhostAnimationData *animData, const BehaviorScript *behavior, struct MarioState *m, s16 frame, s16 level, s16 area) {
    struct Object *ghost = obj_get_time_trials_ghost(behavior);

    // If timer reached its max or frame data is ended, unload the ghost
    if (frame >= TIME_TRIALS_MAX_ALLOWED_TIME || time_trials_is_end_frame(ghostData, frame)) {
        if (ghost != NULL) {
            obj_mark_for_deletion(ghost);
        }
        return;
    }

    // Spawn ghost if not loaded
    if (ghost == NULL) {
        ghost = spawn_object(m->marioObj, 0, behavior);
    }

    // Hide ghost if disabled, on first frame or its level or area differs from Mario
    if (frame == 0 || ghostData[frame].level != (u8) level || ghostData[frame].area != (u8) area) {
        ghost->header.gfx.node.flags |= GRAPH_RENDER_INVISIBLE;
        return;
    }

    // Update ghost
    ghost->hitboxRadius           = 0;
    ghost->hitboxHeight           = 0;
    ghost->oOpacity               = 160;
    ghost->oPosX                  = (f32) ghostData[frame].posX;
    ghost->oPosY                  = (f32) ghostData[frame].posY;
    ghost->oPosZ                  = (f32) ghostData[frame].posZ;
    ghost->oFaceAnglePitch        = (s16) ghostData[frame].pitch  * 0x100;
    ghost->oFaceAngleYaw          = (s16) ghostData[frame].yaw    * 0x100;
    ghost->oFaceAngleRoll         = (s16) ghostData[frame].roll   * 0x100;
    ghost->oMoveAnglePitch        = (s16) ghostData[frame].pitch  * 0x100;
    ghost->oMoveAngleYaw          = (s16) ghostData[frame].yaw    * 0x100;
    ghost->oMoveAngleRoll         = (s16) ghostData[frame].roll   * 0x100;
    ghost->header.gfx.pos[0]      = (f32) ghostData[frame].posX;
    ghost->header.gfx.pos[1]      = (f32) ghostData[frame].posY;
    ghost->header.gfx.pos[2]      = (f32) ghostData[frame].posZ;
    ghost->header.gfx.angle[0]    = (s16) ghostData[frame].pitch  * 0x100;
    ghost->header.gfx.angle[1]    = (s16) ghostData[frame].yaw    * 0x100;
    ghost->header.gfx.angle[2]    = (s16) ghostData[frame].roll   * 0x100;
    ghost->header.gfx.scale[0]    = (f32) ghostData[frame].scaleX / 100.f;
    ghost->header.gfx.scale[1]    = (f32) ghostData[frame].scaleY / 100.f;
    ghost->header.gfx.scale[2]    = (f32) ghostData[frame].scaleZ / 100.f;
    ghost->header.gfx.node.flags &= ~GRAPH_RENDER_INVISIBLE;
    ghost->header.gfx.sharedChild = time_trials_get_ghost_mario_graph_node(animData->color);
    time_trials_ghost_update_animation(animData, ghost, m, (u16) ghostData[frame].animIndex, (u16) ghostData[frame].animFrame);
}

static void time_trials_record_ghost(struct MarioState *m, s32 frame) {
    if (frame < 0 || frame >= TIME_TRIALS_MAX_ALLOWED_TIME) {
        return;
    }

    if (m->marioObj && m->marioObj->header.gfx.unk38.curAnim) {
        sTimeTrialGhostDataW[frame].posX      = (s16) m->marioObj->header.gfx.pos[0];
        sTimeTrialGhostDataW[frame].posY      = (s16) m->marioObj->header.gfx.pos[1];
        sTimeTrialGhostDataW[frame].posZ      = (s16) m->marioObj->header.gfx.pos[2];
        sTimeTrialGhostDataW[frame].pitch     = (s8) (m->marioObj->header.gfx.angle[0] / 0x100);
        sTimeTrialGhostDataW[frame].yaw       = (s8) (m->marioObj->header.gfx.angle[1] / 0x100);
        sTimeTrialGhostDataW[frame].roll      = (s8) (m->marioObj->header.gfx.angle[2] / 0x100);
        sTimeTrialGhostDataW[frame].scaleX    = (u8) MAX(0, MIN(255, m->marioObj->header.gfx.scale[0] * 100.f));
        sTimeTrialGhostDataW[frame].scaleY    = (u8) MAX(0, MIN(255, m->marioObj->header.gfx.scale[1] * 100.f));
        sTimeTrialGhostDataW[frame].scaleZ    = (u8) MAX(0, MIN(255, m->marioObj->header.gfx.scale[2] * 100.f));
        sTimeTrialGhostDataW[frame].animIndex = (u8) time_trials_ghost_retrieve_current_animation_index(m);
        sTimeTrialGhostDataW[frame].animFrame = (u8) MAX(0, MIN(255, MIN(m->marioObj->header.gfx.unk38.animFrame, m->marioObj->header.gfx.unk38.curAnim->unk08 - 1)));
        sTimeTrialGhostDataW[frame].level     = (u8) gCurrLevelNum;
        sTimeTrialGhostDataW[frame].area      = (u8) gCurrAreaIndex;
    } else if (frame > 0) {
        sTimeTrialGhostDataW[frame]           = sTimeTrialGhostDataW[frame - 1];
    }
}

static void time_trials_ghost_unload(const BehaviorScript *behavior) {
    struct Object *ghost = obj_get_time_trials_ghost(behavior);
    if (ghost != NULL) {
        obj_mark_for_deletion(ghost);
    }
}

//
// SM64 Strings
//

static u8 *time_trials_to_sm64_string(const char *str) {
    u8 *str64 = calloc(strlen(str) + 1, sizeof(u8));
    for (int k = 0;; str++) {
        if (*str == 0) {                str64[k++] = 0xFF; break; }
        if (*str >= '0' && *str <= '9') str64[k++] = (u8) (*str - '0' + 0);
        if (*str >= 'A' && *str <= 'Z') str64[k++] = (u8) (*str - 'A' + 10);
        if (*str >= 'a' && *str <= 'z') str64[k++] = (u8) (*str - 'a' + 36);
        if (*str == '\'')               str64[k++] = (u8) (62);
        if (*str == '.')                str64[k++] = (u8) (63);
        if (*str == ':')                str64[k++] = (u8) (230);
        if (*str == ',')                str64[k++] = (u8) (111);
        if (*str == '-')                str64[k++] = (u8) (159);
        if (*str == '"')                str64[k++] = (u8) (246);
        if (*str == '@')                str64[k++] = (u8) (249); // coin
        if (*str == '_')                str64[k++] = (u8) (253); // empty star
        if (*str == '+')                str64[k++] = (u8) (250); // star filled
        if (*str == '*')                str64[k++] = (u8) (251); // x
        if (*str == ' ')                str64[k++] = (u8) (158); // space
    }
    return str64;
}

//
// Int to String
//

static u8 *time_trials_time_to_string(s16 time) {
    if (time == TIME_TRIALS_UNDEFINED_TIME) {
        return time_trials_to_sm64_string("-'--\"--");
    }

    char buffer[32];
    sprintf(buffer, "%01d'%02d\"%02d", (time / 1800) % 10, (time / 30) % 60, ((time * 10) / 3) % 100);
    return time_trials_to_sm64_string(buffer);
}

static u8 *time_trials_course_time_to_string(s32 coursetime) {
    if (coursetime == TIME_TRIALS_UNDEFINED_TIME) {
        return time_trials_to_sm64_string("-'--\"--");
    }

    char buffer[32];
    sprintf(buffer, "%02d'%02d\"%02d", (coursetime / 1800) % 100, (coursetime / 30) % 60, ((coursetime * 10) / 3) % 100);
    return time_trials_to_sm64_string(buffer);
}

static u8 *time_trials_total_time_to_string(s32 totaltime) {
    if (totaltime == TIME_TRIALS_UNDEFINED_TIME) {
        return time_trials_to_sm64_string("--:--:--.--");
    }

    char buffer[32];
    sprintf(buffer, "%02d:%02d:%02d.%02d", (totaltime / 1080000) % 100, (totaltime / 1800) % 60, (totaltime / 30) % 60, ((totaltime * 10) / 3) % 100);
    return time_trials_to_sm64_string(buffer);
}

//
// Render
//

extern u8 gDialogCharWidths[256];
static s16 time_trials_get_string_length(const u8 *str) {
    s16 length = 0;
    for (; *str != DIALOG_CHAR_TERMINATOR; ++str) {
        length += gDialogCharWidths[*str];
    }
    return length;
}

enum { screen_left_edge = 0, screen_center = 1, screen_right_edge = 2 };
enum { align_left = 0, align_center = 1, align_right = 2 };
enum { color_white = 0, color_gray, color_yellow };
static void time_trials_render_string(s16 dx, s16 y, u8 *str, s8 origin, s8 alignment, s8 color) {

    // Compute x position from origin and alignment
    s16 x = 0;
    s16 l = time_trials_get_string_length(str);
    s8 mode = origin * 10 + alignment;
    switch (mode) {
        case 00: x = GFX_DIMENSIONS_FROM_LEFT_EDGE (dx);         break; // screen_left_edge, align_left
        case 01: x = GFX_DIMENSIONS_FROM_LEFT_EDGE (dx - l / 2); break; // screen_left_edge, align_center
        case 02: x = GFX_DIMENSIONS_FROM_LEFT_EDGE (dx - l);     break; // screen_left_edge, align_right
        case 10: x = SCREEN_WIDTH / 2 +            (dx);         break; // screen_center, align_left
        case 11: x = SCREEN_WIDTH / 2 +            (dx - l / 2); break; // screen_center, align_center
        case 12: x = SCREEN_WIDTH / 2 +            (dx - l);     break; // screen_center, align_right
        case 20: x = GFX_DIMENSIONS_FROM_RIGHT_EDGE(dx);         break; // screen_right_edge, align_left
        case 21: x = GFX_DIMENSIONS_FROM_RIGHT_EDGE(dx + l / 2); break; // screen_right_edge, align_center
        case 22: x = GFX_DIMENSIONS_FROM_RIGHT_EDGE(dx + l);     break; // screen_right_edge, align_right
    }

    // Render characters
    static const u8 colors[][3] = {
        { 0xff, 0xff, 0xff },
        { 0xa0, 0xa0, 0xa0 },
        { 0xff, 0xff, 0x00 },
    };
    void **fontLUT = (void **) segmented_to_virtual(main_font_lut);
    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    gDPSetEnvColor(gDisplayListHead++, colors[color][0], colors[color][1], colors[color][2], gDialogTextAlpha);
    create_dl_translation_matrix(MENU_MTX_PUSH, x, y, 0);
    for (u8 *p = str; *p != DIALOG_CHAR_TERMINATOR; ++p) {
        if (*p != DIALOG_CHAR_SPACE) {
            void *tex = segmented_to_virtual(fontLUT[*p]);
            gDPPipeSync(gDisplayListHead++);
            gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_IA, G_IM_SIZ_16b, 1, VIRTUAL_TO_PHYSICAL(tex));
            gSPDisplayList(gDisplayListHead++, dl_ia_text_tex_settings);
        }
        create_dl_translation_matrix(MENU_MTX_NOPUSH, gDialogCharWidths[*p], 0, 0);
    }
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);

    // Free the string after rendering it
    free(str);
}

static void time_trials_render_centered_box(s32 y, s32 w, s32 h) {
    Mtx *matrix = (Mtx *) alloc_display_list(sizeof(Mtx));
    if (!matrix) return;
    create_dl_translation_matrix(MENU_MTX_PUSH, (SCREEN_WIDTH - w) / 2, y + h, 0);
    guScale(matrix, (f32) w / 130.f, (f32) h / 80.f, 1.f);
    gSPMatrix(gDisplayListHead++, VIRTUAL_TO_PHYSICAL(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, 105);
    gSPDisplayList(gDisplayListHead++, dl_draw_text_bg_box);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
}

/*
Position:
Title
1 5
2 6
3 7
4 8
C T
*/

#define TIME_TRIALS_BOX_H_MARGIN    (22.5f * sqr(gfx_current_dimensions.aspect_ratio))
#define TIME_TRIALS_BOX_V_MARGIN    (40.f)
#define TIME_TRIALS_TEXT_H_MARGIN   (9.f * gfx_current_dimensions.aspect_ratio)
#define TIME_TRIALS_TEXT_V_MARGIN   (12.f)

struct RenderParams { const char *label; bool starGet; s16 time; };
static const struct RenderParams sNoParam = { NULL, 0, 0 };

static void time_trials_render_pause_times(struct RenderParams params[8], const char *title, s32 courseTime, s32 totalTime) {

    // Title
    time_trials_render_string(0, SCREEN_HEIGHT - TIME_TRIALS_BOX_V_MARGIN - TIME_TRIALS_TEXT_V_MARGIN - 14, time_trials_to_sm64_string(title), screen_center, align_center, color_white);

    // Stars
    for (s32 i = 0; i != 8; ++i) {
        if (params[i].label == NULL) continue;
        s16 dx, y = (SCREEN_HEIGHT / 2) + ((TIME_TRIALS_TEXT_V_MARGIN / 2) + 10 + TIME_TRIALS_TEXT_V_MARGIN) - (10 + TIME_TRIALS_TEXT_V_MARGIN) * (i % 4) - 3;
        u8 color = ((strcmp(params[i].label, "ALL STARS") == 0) ? color_yellow : color_white);

        // Label
        dx = TIME_TRIALS_TEXT_H_MARGIN + (i < 4 ? TIME_TRIALS_BOX_H_MARGIN : 0);
        time_trials_render_string(dx, y, time_trials_to_sm64_string(params[i].label), (i < 4 ? screen_left_edge : screen_center), align_left, color);

        // Time
        bool undefined = (params[i].time == TIME_TRIALS_UNDEFINED_TIME);
        dx = (TIME_TRIALS_TEXT_H_MARGIN + (i < 4 ? 0 : TIME_TRIALS_BOX_H_MARGIN)) * (i < 4 ? -1 : +1);
        time_trials_render_string(dx, y, time_trials_time_to_string(params[i].time), (i < 4 ? screen_center : screen_right_edge), align_right, undefined ? color_gray : color);

        // Star icon
        bool collected = (params[i].starGet);
        dx = (TIME_TRIALS_TEXT_H_MARGIN + (i < 4 ? 0 : TIME_TRIALS_BOX_H_MARGIN) + 48) * (i < 4 ? -1 : +1);
        time_trials_render_string(dx, y, time_trials_to_sm64_string(params[i].starGet ? "+" : "_"), (i < 4 ? screen_center : screen_right_edge), align_right, collected ? color : color_gray);
    }

    // Course time
    {
        bool undefined = (courseTime == TIME_TRIALS_UNDEFINED_TIME);
        time_trials_render_string(TIME_TRIALS_TEXT_H_MARGIN + TIME_TRIALS_BOX_H_MARGIN, TIME_TRIALS_BOX_V_MARGIN + TIME_TRIALS_TEXT_V_MARGIN - 3, time_trials_to_sm64_string("COURSE"), screen_left_edge, align_left, color_white);
        time_trials_render_string(-TIME_TRIALS_TEXT_H_MARGIN, TIME_TRIALS_BOX_V_MARGIN + TIME_TRIALS_TEXT_V_MARGIN - 3, time_trials_course_time_to_string(courseTime), screen_center, align_right, undefined ? color_gray : color_white);
    }
    
    // Total time
    {
        bool undefined = (totalTime == TIME_TRIALS_UNDEFINED_TIME);
        time_trials_render_string(TIME_TRIALS_TEXT_H_MARGIN, TIME_TRIALS_BOX_V_MARGIN + TIME_TRIALS_TEXT_V_MARGIN - 3, time_trials_to_sm64_string("TOTAL"), screen_center, align_left, color_white);
        time_trials_render_string(TIME_TRIALS_TEXT_H_MARGIN + TIME_TRIALS_BOX_H_MARGIN, TIME_TRIALS_BOX_V_MARGIN + TIME_TRIALS_TEXT_V_MARGIN - 3, time_trials_total_time_to_string(totalTime), screen_right_edge, align_right, undefined ? color_gray : color_white);
    }
}

static void time_trials_render_pause_course_times(s32 fileIndex, s32 courseNum) {
    static const char *sCourseNames[] = {
        "1 - BOB-OMB BATTLEFIELD",
        "2 - WHOMP FORTRESS",
        "3 - JOLLY ROGER BAY",
        "4 - COOL, COOL MOUNTAIN",
        "5 - BIG BOO'S HAUNT",
        "6 - HAZY MAZE CAVE",
        "7 - LETHAL LAVA LAND",
        "8 - SHIFTING SAND LAND",
        "9 - DIRE, DIRE DOCKS",
        "10 - SNOWMAN'S LAND",
        "11 - WET-DRY WORLD",
        "12 - TALL, TALL MOUNTAIN",
        "13 - TINY-HUGE ISLAND",
        "14 - TICK TOCK CLOCK",
        "15 - RAINBOW RIDE"
    };

    u8 starFlags = save_file_get_star_flags(fileIndex, courseNum - COURSE_MIN);
    u8 allStars  = time_trials_get_time(fileIndex, courseNum, 7) != TIME_TRIALS_UNDEFINED_TIME;

    time_trials_render_pause_times((struct RenderParams[]) {
        (struct RenderParams) { "STAR 1",    (starFlags >> 0) & 1, time_trials_get_time(fileIndex, courseNum, 0) }, 
        (struct RenderParams) { "STAR 2",    (starFlags >> 1) & 1, time_trials_get_time(fileIndex, courseNum, 1) },
        (struct RenderParams) { "STAR 3",    (starFlags >> 2) & 1, time_trials_get_time(fileIndex, courseNum, 2) },
        OMM_STARS_NON_STOP ?
        (struct RenderParams) { "ALL STARS", (allStars)      != 0, time_trials_get_time(fileIndex, courseNum, 7) } :
        sNoParam,
        (struct RenderParams) { "STAR 4",    (starFlags >> 3) & 1, time_trials_get_time(fileIndex, courseNum, 3) },
        (struct RenderParams) { "STAR 5",    (starFlags >> 4) & 1, time_trials_get_time(fileIndex, courseNum, 4) },
        (struct RenderParams) { "STAR 6",    (starFlags >> 5) & 1, time_trials_get_time(fileIndex, courseNum, 5) },
        (struct RenderParams) { "STAR @",    (starFlags >> 6) & 1, time_trials_get_time(fileIndex, courseNum, 6) },
    },
        sCourseNames[courseNum - COURSE_MIN],
        time_trials_get_course_time(fileIndex, courseNum),
        time_trials_get_total_time(fileIndex)
    );
}

static void time_trials_render_pause_bowser_times(s32 fileIndex) {
    u8 bitdwStarFlags = save_file_get_star_flags(fileIndex, COURSE_BITDW - COURSE_MIN);
    u8 bitfsStarFlags = save_file_get_star_flags(fileIndex, COURSE_BITFS - COURSE_MIN);
    u8 bitsStarFlags  = save_file_get_star_flags(fileIndex, COURSE_BITS - COURSE_MIN);
    u8 bitdwKeyFlag   = save_file_get_flags() & (SAVE_FLAG_HAVE_KEY_1 | SAVE_FLAG_UNLOCKED_BASEMENT_DOOR);
    u8 bitfsKeyFlag   = save_file_get_flags() & (SAVE_FLAG_HAVE_KEY_2 | SAVE_FLAG_UNLOCKED_UPSTAIRS_DOOR);
    u8 bitsGrandStar  = time_trials_get_time(fileIndex, COURSE_BITS, 1) != TIME_TRIALS_UNDEFINED_TIME;

    time_trials_render_pause_times((struct RenderParams[]) {
        (struct RenderParams) { "STAR 1", (bitdwStarFlags >> 0) & 1, time_trials_get_time(fileIndex, COURSE_BITDW, 0) }, 
        (struct RenderParams) { "STAR 2", (bitfsStarFlags >> 0) & 1, time_trials_get_time(fileIndex, COURSE_BITFS, 0) },
        (struct RenderParams) { "STAR 3", (bitsStarFlags  >> 0) & 1, time_trials_get_time(fileIndex, COURSE_BITS,  0) },
        sNoParam,
        (struct RenderParams) { "KEY 1",  (bitdwKeyFlag)       != 0, time_trials_get_time(fileIndex, COURSE_BITDW, 1) },
        (struct RenderParams) { "KEY 2",  (bitfsKeyFlag)       != 0, time_trials_get_time(fileIndex, COURSE_BITFS, 1) },
        (struct RenderParams) { "FINAL",  (bitsGrandStar)      != 0, time_trials_get_time(fileIndex, COURSE_BITS,  1) },
        sNoParam,
    },
        "BOWSER COURSES",
        time_trials_get_bowser_time(fileIndex),
        time_trials_get_total_time(fileIndex)
    );
}

static void time_trials_render_pause_secret_times(s32 fileIndex) {
    u8 totwcStarFlags = save_file_get_star_flags(fileIndex, COURSE_TOTWC - COURSE_MIN);
    u8 vcutmStarFlags = save_file_get_star_flags(fileIndex, COURSE_VCUTM - COURSE_MIN);
    u8 cotmcStarFlags = save_file_get_star_flags(fileIndex, COURSE_COTMC - COURSE_MIN);
    u8 pssStarFlags   = save_file_get_star_flags(fileIndex, COURSE_PSS - COURSE_MIN);
    u8 saStarFlags    = save_file_get_star_flags(fileIndex, COURSE_SA - COURSE_MIN);
    u8 wmotrStarFlags = save_file_get_star_flags(fileIndex, COURSE_WMOTR - COURSE_MIN);

    time_trials_render_pause_times((struct RenderParams[]) {
        (struct RenderParams) { "PSS 1",  (pssStarFlags   >> 0) & 1, time_trials_get_time(fileIndex, COURSE_PSS,   0) },
        (struct RenderParams) { "PSS 2",  (pssStarFlags   >> 1) & 1, time_trials_get_time(fileIndex, COURSE_PSS,   1) },
        (struct RenderParams) { "SA",     (saStarFlags    >> 0) & 1, time_trials_get_time(fileIndex, COURSE_SA,    0) },
        (struct RenderParams) { "WMOTR",  (wmotrStarFlags >> 0) & 1, time_trials_get_time(fileIndex, COURSE_WMOTR, 0) },
        (struct RenderParams) { "TOTWC",  (totwcStarFlags >> 0) & 1, time_trials_get_time(fileIndex, COURSE_TOTWC, 0) }, 
        (struct RenderParams) { "VCUTM",  (vcutmStarFlags >> 0) & 1, time_trials_get_time(fileIndex, COURSE_VCUTM, 0) },
        (struct RenderParams) { "COTMC",  (cotmcStarFlags >> 0) & 1, time_trials_get_time(fileIndex, COURSE_COTMC, 0) },
        sNoParam,
    },
        "SECRET COURSES",
        time_trials_get_secret_time(fileIndex),
        time_trials_get_total_time(fileIndex)
    );
}

Gfx *gTimeTableDisplayListHead;
static bool sSetIndexToCurrentCourse = false;
extern void create_dl_rotation_matrix(s8 pushOp, f32 a, f32 x, f32 y, f32 z);
bool time_trials_render_time_table(s8 *index) {
    if (!sTimeTrialsEnabled) {
        return false;
    }

    // Handle menu scrolling
    static s8 sCastleHubCurrentCourse = 0;
    if (sSetIndexToCurrentCourse) {
        if (gCurrCourseNum == COURSE_NONE) *index = sCastleHubCurrentCourse;
        else if (gCurrCourseNum >= COURSE_BITDW && gCurrCourseNum <= COURSE_BITS) *index = COURSE_STAGES_COUNT;
        else if (gCurrCourseNum > COURSE_BITS) *index = COURSE_STAGES_COUNT + 1;
        else *index = gCurrCourseNum - 1;
        sSetIndexToCurrentCourse = false;
    }
    handle_menu_scrolling(MENU_SCROLL_VERTICAL, index, -1, COURSE_STAGES_COUNT + 2);
    if (*index == COURSE_STAGES_COUNT + 2) *index = 0;
    if (*index == -1) *index = COURSE_STAGES_COUNT + 1;
    sCastleHubCurrentCourse = *index;

    // Render the colorful "PAUSE"
    gDisplayListHead = gTimeTableDisplayListHead;
    u8 *pause = time_trials_to_sm64_string("TIME TRIALS");
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gDialogTextAlpha);
    print_hud_lut_string(HUD_LUT_GLOBAL, SCREEN_WIDTH / 2 - 64, 8, pause);
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);
    free(pause);

    // Render the black box behind the text
    time_trials_render_centered_box(TIME_TRIALS_BOX_V_MARGIN, GFX_DIMENSIONS_FROM_RIGHT_EDGE(TIME_TRIALS_BOX_H_MARGIN) - GFX_DIMENSIONS_FROM_LEFT_EDGE(TIME_TRIALS_BOX_H_MARGIN), SCREEN_HEIGHT - 2 * TIME_TRIALS_BOX_V_MARGIN);
    create_dl_translation_matrix(MENU_MTX_PUSH, SCREEN_WIDTH / 2 + 8, SCREEN_HEIGHT - TIME_TRIALS_BOX_V_MARGIN + 4, 0);
    create_dl_rotation_matrix(MENU_MTX_NOPUSH, 90.0f, 0, 0, 1.0f);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gDialogTextAlpha);
    gSPDisplayList(gDisplayListHead++, dl_draw_triangle);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
    create_dl_translation_matrix(MENU_MTX_PUSH, SCREEN_WIDTH / 2 - 8, TIME_TRIALS_BOX_V_MARGIN - 4, 0);
    create_dl_rotation_matrix(MENU_MTX_NOPUSH, 270.0f, 0, 0, 1.0f);
    gSPDisplayList(gDisplayListHead++, dl_draw_triangle);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);

    // Render the text
    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gDialogTextAlpha);
    switch (*index) {
        case COURSE_STAGES_COUNT + 1: time_trials_render_pause_secret_times(gCurrSaveFileNum - 1); break;
        case COURSE_STAGES_COUNT:     time_trials_render_pause_bowser_times(gCurrSaveFileNum - 1); break;
        default:                      time_trials_render_pause_course_times(gCurrSaveFileNum - 1, *index + 1); break;
    }
    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
    return true;
}

//
// HUD
//

static s16 time_trials_get_x_from_center(const u8 *str64) {
    s16 len = 0;
    while (*str64 != 0xFF) {
        len += (*str64 == GLOBAL_CHAR_SPACE ? 8 : 12);
        str64++;
    }
    return (SCREEN_WIDTH / 2) - len;
}

static void time_trials_render_timer(s16 y, const char *text, s16 time, u8 colorFade) {
    u8 *s = time_trials_to_sm64_string(text);
    s16 x = time_trials_get_x_from_center(s);
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    gDPSetEnvColor(gDisplayListHead++, colorFade, colorFade, colorFade, 255);
    print_hud_lut_string(HUD_LUT_GLOBAL,                x -  6, y,     s);
    print_hud_lut_string(HUD_LUT_GLOBAL, SCREEN_WIDTH / 2 +  6, y,     time_trials_int_to_sm64_string( (time / 1800)    % 60,  "%01d"));
    print_hud_lut_string(HUD_LUT_GLOBAL, SCREEN_WIDTH / 2 + 26, y,     time_trials_int_to_sm64_string( (time / 30)      % 60,  "%02d"));
    print_hud_lut_string(HUD_LUT_GLOBAL, SCREEN_WIDTH / 2 + 60, y,     time_trials_int_to_sm64_string(((time * 10) / 3) % 100, "%02d"));
    print_hud_lut_string(HUD_LUT_GLOBAL, SCREEN_WIDTH / 2 + 16, y - 8, (const u8 *) (const u8 []) { 56, 255 });
    print_hud_lut_string(HUD_LUT_GLOBAL, SCREEN_WIDTH / 2 + 51, y - 8, (const u8 *) (const u8 []) { 57, 255 });
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
    free(s);
}

void time_trials_render_star_select_time(s32 starIndex) {
    if (sTimeTrialsEnabled) {
        s16 time = time_trials_get_time(gCurrSaveFileNum - 1, gCurrCourseNum, starIndex);
        if (time != TIME_TRIALS_UNDEFINED_TIME) {
            time_trials_render_timer(12, "HI SCORE", time, 255);
        }
    }
}

//
// Update
//

static s32 time_trials_get_ghost_data_to_load(s32 courseNum, s32 starIndex) {
    if (courseNum >= COURSE_BOB   && courseNum <= COURSE_RR)   return MAX(0, MIN(starIndex, 5));
    if (courseNum >= COURSE_BITDW && courseNum <= COURSE_BITS) return 1;
    if (courseNum == COURSE_PSS)                               return 0;
    return 0;
}

static s32 time_trials_get_special_ghost_data_to_load(s32 courseNum) {
    if (courseNum >= COURSE_BOB   && courseNum <= COURSE_RR)   return 6;
    if (courseNum >= COURSE_BITDW && courseNum <= COURSE_BITS) return 0;
    if (courseNum == COURSE_PSS)                               return 1;
    return -1;
}

static bool time_trials_should_start_timer(struct MarioState *m) {
    switch (m->action) {
        case ACT_SPAWN_SPIN_AIRBORNE:       return (gCurrCourseNum >= COURSE_BOB && gCurrCourseNum <= COURSE_STAGES_MAX);
        case ACT_SPAWN_NO_SPIN_AIRBORNE:    return (gCurrCourseNum >  COURSE_STAGES_MAX);
        case ACT_FLYING:                    return (gCurrCourseNum == COURSE_TOTWC && m->actionArg == 2 && m->actionTimer++ == 0);
        case ACT_WATER_IDLE:                return (gCurrCourseNum == COURSE_SA && m->actionArg == 1 && m->actionTimer++ == 0);
    }
    return false;
}

void time_trials_update(bool isPaused) {
    
    // Hold L and press B to enable/disable Time Trials during the pause
    if (isPaused) {
        if ((gControllers[0].buttonDown & L_TRIG) && (gControllers[0].buttonPressed & B_BUTTON)) {
            sTimeTrialsEnabled = !sTimeTrialsEnabled;
            if (sTimeTrialsEnabled) {
                play_sound(SOUND_MENU_STAR_SOUND, gDefaultSoundArgs);
            }
        }
    }
    if (!sTimeTrialsEnabled) {
        sTimeTrialTimer = TIME_TRIALS_MAX_ALLOWED_TIME;
        sTimeTrialTimerState = TT_TIMER_DISABLED;
        time_trials_ghost_unload(bhvTimeTrialGhost1);
        time_trials_ghost_unload(bhvTimeTrialGhost2);
        return;
    }

    // Pause
    static bool sIsPaused = false;
    if (isPaused && !sIsPaused) {
        sSetIndexToCurrentCourse = true;
    }
    sIsPaused = isPaused;

    // Update running ghost, record ghost data, and advance timer if it's running
    if (sTimeTrialTimerState == TT_TIMER_RUNNING) {
        sTimeTrialHiScore = false;
            
        // Update ghosts and advance timer if not paused
        if (!isPaused) {
            time_trials_update_ghost(sTimeTrialGhostDataR1, &sGhostAnimData1, bhvTimeTrialGhost1, gMarioState, sTimeTrialTimer, gCurrLevelNum, gCurrAreaIndex);
            time_trials_update_ghost(sTimeTrialGhostDataR2, &sGhostAnimData2, bhvTimeTrialGhost2, gMarioState, sTimeTrialTimer, gCurrLevelNum, gCurrAreaIndex);
            time_trials_record_ghost(gMarioState, sTimeTrialTimer);
            sTimeTrialTimer = MIN(sTimeTrialTimer + 1, TIME_TRIALS_MAX_ALLOWED_TIME);
        }
    }

    // Runs timer if course is TTable
    static s16 sLoadedSvFileNum = -1;
    static s16 sLoadedCourseNum = -1;
    static s16 sLoadedStarIndex = -1;
    if (time_trials_get_slot_index(gCurrCourseNum, 0) != -1) {
        if (sTimeTrialTimerState != TT_TIMER_STOPPED) {
            sTimeTrialTimerState = TT_TIMER_RUNNING;
            sTimeTrialHiScore = false;

            // Start timer and init ghost if a course entry is detected
            if (time_trials_should_start_timer(gMarioState)) {
                bzero(sTimeTrialGhostDataW, sizeof(sTimeTrialGhostDataW));
                sTimeTrialTimer = 0;

                // Load ghosts only if not already loaded
                s32 starIndex = time_trials_get_ghost_data_to_load(gCurrCourseNum, gCurrActNum - 1);
                if ((sLoadedSvFileNum != gCurrSaveFileNum) || (sLoadedCourseNum != gCurrCourseNum) || (sLoadedStarIndex != starIndex)) {

                    // Default ghost (blue)
                    // All Stars ghost (pink)
                    if (OMM_STARS_NON_STOP && gCurrCourseNum >= COURSE_BOB && gCurrCourseNum <= COURSE_RR) {
                        starIndex = 7;
                    }
                    s32 slotIndex = time_trials_get_slot_index(gCurrCourseNum, starIndex);
                    time_trials_read_ghost_data(gCurrSaveFileNum - 1, gCurrCourseNum - 1, starIndex, slotIndex, sTimeTrialGhostDataR1);
                    sGhostAnimData1.color = (starIndex == 7 ? GHOST_MARIO_ALL_STARS : GHOST_MARIO_DEFAULT);
                    sGhostAnimData1.prevValidAnimIndex = 0xFFFF;

                    // Special ghost
                    // Bowser Red coins (red)
                    // PSS 2 (green)
                    // 100 coins (gold)
                    s32 specialIndex = time_trials_get_special_ghost_data_to_load(gCurrCourseNum);
                    if (specialIndex != -1) {
                        slotIndex = time_trials_get_slot_index(gCurrCourseNum, specialIndex);
                        time_trials_read_ghost_data(gCurrSaveFileNum - 1, gCurrCourseNum - 1, specialIndex, slotIndex, sTimeTrialGhostDataR2);
                        if (specialIndex == 0) sGhostAnimData2.color = GHOST_MARIO_RED_COINS;
                        if (specialIndex == 1) sGhostAnimData2.color = GHOST_MARIO_PSS_2;
                        if (specialIndex == 6) sGhostAnimData2.color = GHOST_MARIO_100_COINS;
                        sGhostAnimData2.prevValidAnimIndex = 0xFFFF;
                    }

                    sLoadedSvFileNum = gCurrSaveFileNum;
                    sLoadedCourseNum = gCurrCourseNum;
                    sLoadedStarIndex = starIndex;
                }
            }

            // Stop timer and save time when Mario collects a star
            struct MarioState *m = gMarioState;
            if (m->action == ACT_STAR_DANCE_EXIT ||
                m->action == ACT_STAR_DANCE_NO_EXIT ||
                m->action == ACT_STAR_DANCE_WATER ||
                m->action == ACT_FALL_AFTER_STAR_GRAB ||
                m->action == ACT_JUMBO_STAR_CUTSCENE) {

                // Force Mario to leave the course if he collects a main course star
                // Ignore this if OMM Non-Stop mode is enabled
                if (gCurrCourseNum >= COURSE_MIN && gCurrCourseNum <= COURSE_STAGES_MAX) {
                    if (!OMM_STARS_NON_STOP || OMM_ALL_STARS) {
                        drop_queued_background_music();
                        fadeout_level_music(126);
                        m->actionArg = 0;
                    }
                }

                // Save the time
                if (sTimeTrialTimerState == TT_TIMER_RUNNING) {
                    s32 starIndex = gLastCompletedStarNum - 1;

                    // Bowser Key or Grand Star
                    if (gCurrLevelNum == LEVEL_BOWSER_1 ||
                        gCurrLevelNum == LEVEL_BOWSER_2 ||
                        gCurrLevelNum == LEVEL_BOWSER_3) {
                        starIndex = 1;
                        m->actionArg = 0;
                    }

                    // Write time and ghost data if new record
                    s32 slotIndex = time_trials_get_slot_index(gCurrCourseNum, starIndex);
                    if (slotIndex != -1) {
                        s16 t = *time_trials_time(gCurrSaveFileNum - 1, slotIndex);
                        if (t == TIME_TRIALS_UNDEFINED_TIME || t > sTimeTrialTimer) {
                            time_trials_write_ghost_data(gCurrSaveFileNum - 1, gCurrCourseNum - 1, starIndex);
                            sTimeTrialHiScore = true;
                        } else {
                            sTimeTrialHiScore = false;
                        }
                    }

                    // All Stars
                    if (OMM_ALL_STARS) {
                        slotIndex = time_trials_get_slot_index(gCurrCourseNum, 7);
                        if (slotIndex != -1) {
                            s16 t = *time_trials_time(gCurrSaveFileNum - 1, slotIndex);
                            if (t == TIME_TRIALS_UNDEFINED_TIME || t > sTimeTrialTimer) {
                                time_trials_write_ghost_data(gCurrSaveFileNum - 1, gCurrCourseNum - 1, 7);
                                sTimeTrialHiScore = true;
                            } else {
                                sTimeTrialHiScore = false;
                            }
                        }
                    }

                    // Stop the timer if Mario leaves the course
                    if (m->actionArg == 0) {
                        sTimeTrialTimerState = TT_TIMER_STOPPED;
                    }
                }
            }
        }
    } else {
        sTimeTrialTimer = TIME_TRIALS_MAX_ALLOWED_TIME;
        sTimeTrialTimerState = TT_TIMER_DISABLED;
        sLoadedSvFileNum = -1;
        sLoadedCourseNum = -1;
        sLoadedStarIndex = -1;
    }

    // Render timer
    if (gHudDisplay.flags != HUD_DISPLAY_NONE && configHUD && sTimeTrialTimerState != TT_TIMER_DISABLED) {
        if (sTimeTrialHiScore && sTimeTrialTimerState == TT_TIMER_STOPPED) {
            time_trials_render_timer(SCREEN_HEIGHT - 24, "HI SCORE", sTimeTrialTimer, (sins(gGlobalTimer * 0x1000) * 50.f + 205.f));
        } else if (sTimeTrialTimer > 0) {
            time_trials_render_timer(SCREEN_HEIGHT - 24, "TIME", sTimeTrialTimer, 255);
        }
    }
}
