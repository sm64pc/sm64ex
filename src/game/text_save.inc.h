#include <string.h>
#include <stdio.h>
#include <time.h>
#include "course_table.h"
#include "pc/ini.h"
#include "pc/platform.h"
#include "pc/fs/fs.h"

#define FILENAME_FORMAT "%s/sm64_save_file_%d.sav"
#define NUM_COURSES 15
#define NUM_BONUS_COURSES 10
#define NUM_FLAGS 21
#define NUM_CAP_ON 4

const char *sav_flags[NUM_FLAGS] = {
    "file_exists", "wing_cap", "metal_cap", "vanish_cap", "key_1", "key_2",
    "basement_door", "upstairs_door", "ddd_moved_back", "moat_drained",
    "pps_door", "wf_door", "ccm_door", "jrb_door", "bitdw_door",
    "bitfs_door", "", "", "", "", "50star_door"    // 4 Cap flags are processed in their own section
};

const char *sav_courses[NUM_COURSES] = {
    "bob", "wf", "jrb", "ccm", "bbh", "hmc", "lll",
    "ssl", "ddd", "sl", "wdw", "ttm", "thi", "ttc", "rr"
};

const char *sav_bonus_courses[NUM_BONUS_COURSES] = {
    "bitdw", "bitfs", "bits", "pss", "cotmc",
    "totwc", "vcutm", "wmotr", "sa", "hub"    // hub is Castle Grounds
};

const char *cap_on_types[NUM_CAP_ON] = {
    "ground", "klepto", "ukiki", "mrblizzard"
};

const char *sound_modes[3] = {
    "stereo", "mono", "headset"
};

/* Get current timestamp string */
static void get_timestamp(char* buffer) {
    time_t timer;
    struct tm* tm_info;

    timer = time(NULL);
    tm_info = localtime(&timer);

    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
}

/* Convert 'binary' integer to decimal integer */
static u32 bin_to_int(u32 n) {
    s32 dec = 0, i = 0, rem;
    while (n != 0) {
        rem = n % 10;
        n /= 10;
        dec += rem * (1 << i);
        ++i;
    }
    return dec;
}

/* Convert decimal integer to 'binary' integer */
static u32 int_to_bin(u32 n) {
    s32 bin = 0, rem, i = 1;
    while (n != 0) {
        rem = n % 2;
        n /= 2;
        bin += rem * i;
        i *= 10;
    }
    return bin;
}

/**
 * Write SaveFile and MainMenuSaveData structs to a text-based savefile
 */
static s32 write_text_save(s32 fileIndex) {
    FILE* file;
    struct SaveFile *savedata;
    struct MainMenuSaveData *menudata;
    char filename[SYS_MAX_PATH] = { 0 };
    char value[64];
    u32 i, bit, flags, coins, stars, starFlags;

    if (snprintf(filename, sizeof(filename), FILENAME_FORMAT, fs_writepath, fileIndex) < 0)
        return -1;

    file = fopen(filename, "wt");
    if (file == NULL) {
        printf("Savefile '%s' not found!\n", filename);
        return -1;
    } else
        printf("Saving updated progress to '%s'\n", filename);

    fprintf(file, "# Super Mario 64 save file\n");
    fprintf(file, "# Comment starts with #\n");
    fprintf(file, "# True = 1, False = 0\n");

    get_timestamp(value);
    fprintf(file, "# %s\n", value);

    menudata = &gSaveBuffer.menuData[0];
    fprintf(file, "\n[menu]\n");
    fprintf(file, "coin_score_age = %d\n", menudata->coinScoreAges[fileIndex]);
    
    if (menudata->soundMode == 0) {
        fprintf(file, "sound_mode = %s\n", sound_modes[0]);  // stereo
    }
    else if (menudata->soundMode == 3) {
        fprintf(file, "sound_mode = %s\n", sound_modes[1]);  // mono
    }
    else if (menudata->soundMode == 1) {
        fprintf(file, "sound_mode = %s\n", sound_modes[2]);  // headset
    }
    else {
        printf("Undefined sound mode!");
        return -1;
    }
    
    fprintf(file, "\n[flags]\n");
    for (i = 1; i < NUM_FLAGS; i++) {
        if (strcmp(sav_flags[i], "")) {
            flags = save_file_get_flags();
            flags = (flags & (1 << i));     // Get 'star' flag bit
            flags = (flags) ? 1 : 0;

            fprintf(file, "%s = %d\n", sav_flags[i], flags);
        }
    }

    fprintf(file, "\n[courses]\n");
    for (i = 0; i < NUM_COURSES; i++) {
        stars = save_file_get_star_flags(fileIndex, i);
        coins = save_file_get_course_coin_score(fileIndex, i);
        starFlags = int_to_bin(stars);      // 63 -> 111111
            
        fprintf(file, "%s = \"%d, %07d\"\n", sav_courses[i], coins, starFlags);
    }

    fprintf(file, "\n[bonus]\n");
    for (i = 0; i < NUM_BONUS_COURSES; i++) {
        char *format;

        if (i == NUM_BONUS_COURSES-1) {
            // Process Castle Grounds
            stars = save_file_get_star_flags(fileIndex, -1);
            format = "%05d";
        } else if (i == 3) {
            // Process Princess's Secret Slide
            stars = save_file_get_star_flags(fileIndex, 18);
            format = "%02d";
        } else {
            // Process bonus courses
            stars = save_file_get_star_flags(fileIndex, i+15);
            format = "%d";
        }

        starFlags = int_to_bin(stars);
        if (sprintf(value, format, starFlags) < 0)
            return -1;
        fprintf(file, "%s = %s\n", sav_bonus_courses[i], value);
    }

    fprintf(file, "\n[cap]\n");
    for (i = 0; i < NUM_CAP_ON; i++) {
        flags = save_file_get_flags();
        bit = (1 << (i+16));        // Determine current flag
        flags = (flags & bit);      // Get 'cap' flag bit
        flags = (flags) ? 1 : 0;
        if (flags) {
            fprintf(file, "type = %s\n", cap_on_types[i]);
            break;
        }
    }

    savedata = &gSaveBuffer.files[fileIndex][0];
    switch(savedata->capLevel) {
        case COURSE_SSL:
            fprintf(file, "level = %s\n", "ssl");
            break;
        case COURSE_SL:
            fprintf(file, "level = %s\n", "sl");
            break;
        case COURSE_TTM:
            fprintf(file, "level = %s\n", "ttm");
            break;
        default:
            break;
    }
    if (savedata->capLevel) {
        fprintf(file, "area = %d\n", savedata->capArea);
    }

    // Backup is nessecary for saving recent progress after gameover
    bcopy(&gSaveBuffer.files[fileIndex][0], &gSaveBuffer.files[fileIndex][1],
          sizeof(gSaveBuffer.files[fileIndex][1]));
    
    fclose(file);
    return 1;
}

/**
 * Read gSaveBuffer data from a text-based savefile
 */
static s32 read_text_save(s32 fileIndex) {
    char filename[SYS_MAX_PATH] = { 0 };
    const char *value;
    ini_t *savedata;
    
    u32 i, flag, coins, stars, starFlags;
    u32 capArea;
    
    if (snprintf(filename, sizeof(filename), FILENAME_FORMAT, fs_writepath, fileIndex) < 0)
        return -1;

    savedata = ini_load(filename);
    if (savedata == NULL) {
        return -1;
    } else {
        printf("Loading savefile from '%s'\n", filename);
    }

    ini_sget(savedata, "menu", "coin_score_age", "%d",
                &gSaveBuffer.menuData[0].coinScoreAges[fileIndex]);
    
    value = ini_get(savedata, "menu", "sound_mode");
    if (value) {
        if (strcmp(value, sound_modes[0]) == 0) {
            gSaveBuffer.menuData[0].soundMode = 0;  // stereo
        }
        else if (strcmp(value, sound_modes[1]) == 0) {
            gSaveBuffer.menuData[0].soundMode = 3;  // mono
        }
        else if (strcmp(value, sound_modes[2]) == 0) {
            gSaveBuffer.menuData[0].soundMode = 1;  // headset
        }
    }
    else {
        printf("Invalid 'menu:sound_mode' flag!\n");
        return -1;
    }
    
    for (i = 1; i < NUM_FLAGS; i++) {
        value = ini_get(savedata, "flags", sav_flags[i]);
        if (value) {
            flag = value[0] - '0';  // Flag should be 0 or 1
            if (flag) {
                flag = 1 << i;      // Flags defined in 'save_file' header
                gSaveBuffer.files[fileIndex][0].flags |= flag;
            }
        }
    }

    for (i = 0; i < NUM_COURSES; i++) {
        value = ini_get(savedata, "courses", sav_courses[i]);
        if (value) {
            sscanf(value, "%d, %d", &coins, &stars); 
            starFlags = bin_to_int(stars);      // 111111 -> 63

            save_file_set_star_flags(fileIndex, i, starFlags);
            gSaveBuffer.files[fileIndex][0].courseCoinScores[i] = coins;
        }
    }

    for (i = 0; i < NUM_BONUS_COURSES; i++) {
        value = ini_get(savedata, "bonus", sav_bonus_courses[i]);
        if (value) {
            sscanf(value, "%d", &stars);
            starFlags = bin_to_int(stars);

            if (strlen(value) == 5) {
                // Process Castle Grounds
                save_file_set_star_flags(fileIndex, -1, starFlags);
            } else if (strlen(value) == 2) {
                // Process Princess's Secret Slide
                save_file_set_star_flags(fileIndex, 18, starFlags);
            } else {
                // Process bonus courses
                save_file_set_star_flags(fileIndex, i+15, starFlags);
            }
        }
    }

    for (i = 0; i < NUM_CAP_ON; i++) {
        value = ini_get(savedata, "cap", "type");
        if (value) {
            if (!strcmp(value, cap_on_types[i])) {
                flag = (1 << (16 + i));
                gSaveBuffer.files[fileIndex][0].flags |= flag;
                break;
            }
        }
    }
    
    value = ini_get(savedata, "cap", "level");
    if (value) {
        if (strcmp(value, "ssl") == 0) {
            gSaveBuffer.files[fileIndex][0].capLevel = COURSE_SSL; // ssl
        }
        else if (strcmp(value, "sl") == 0) {
            gSaveBuffer.files[fileIndex][0].capLevel = COURSE_SL; // sl
        }
        else if (strcmp(value, "ttm") == 0) {
            gSaveBuffer.files[fileIndex][0].capLevel = COURSE_TTM; // ttm
        }
        else {
            printf("Invalid 'cap:level' flag!\n");
            return -1;
        }
    }
    
    value = ini_get(savedata, "cap", "area");
    if (value) {
        sscanf(value, "%d", &capArea);
        if (capArea > 1 && capArea < 2) {
            printf("Invalid 'cap:area' flag: %d!\n", capArea);
            return -1;
        }
        else {
            gSaveBuffer.files[fileIndex][0].capArea = capArea; 
        }
    }
    
    // Good, file exists for gSaveBuffer
    gSaveBuffer.files[fileIndex][0].flags |= SAVE_FLAG_FILE_EXISTS;

    // Backup is nessecary for saving recent progress after gameover
    bcopy(&gSaveBuffer.files[fileIndex][0], &gSaveBuffer.files[fileIndex][1],
          sizeof(gSaveBuffer.files[fileIndex][1]));
    
    ini_free(savedata);
    return 0;
}
