#include <string.h>
#include <stdio.h>
#include <time.h>
#include "course_table.h"
#include "pc/platform.h"
#include "pc/ini.h"

#define NUM_COURSES 15
#define NUM_BONUS_COURSES 10
#define NUM_FLAGS 21
#define NUM_CAP_ON 4

/* Flag keys */
const char *sav_flags[NUM_FLAGS] = {
    "file_exists", "wing_cap", "metal_cap", "vanish_cap", "key_1", "key_2",
    "basement_door", "upstairs_door", "ddd_moved_back", "moat_drained",
    "pps_door", "wf_door", "ccm_door", "jrb_door", "bitdw_door",
    "bitfs_door", "", "", "", "", "50star_door"
};

/* Main course keys */
const char *sav_courses[NUM_COURSES] = {
    "bob", "wf", "jrb", "ccm", "bbh", "hmc", "lll",
    "ssl", "ddd", "sl", "wdw", "ttm", "thi", "ttc", "rr"
};

/* Bonus courses keys (including Castle Course) */
const char *sav_bonus_courses[NUM_BONUS_COURSES] = {
    "bitdw", "bitfs", "bits", "pss", "cotmc",
    "totwc", "vcutm", "wmotr", "sa", "hub",
};

/* Mario's cap type keys */
const char *cap_on_types[NUM_CAP_ON] = {
    "ground", "klepto", "ukiki", "mrblizzard",
};

/* Sound modes */
const char *sound_modes[3] = {
    "stereo", "mono", "headset"
};

/* Get current timestamp */
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
 * Write SaveFile and MainMenuSaveData structs to a text-based savefile.
 */
static s32 write_text_save(s32 fileIndex) {
    FILE* file;
    struct SaveFile *savedata;
    struct MainMenuSaveData *menudata;
    char path[SYS_MAX_PATH] = { 0 };
    char value[32] = { 0 };
    u32 i, bit, flags, coins, stars, starFlags;

    /* Load savefile from defined path */
    snprintf(path, sizeof(path), "%s\\sm64_save_file_%d.sav", sys_save_path(), fileIndex);
    file = fopen(path, "wt");
    if (file == NULL) {
        printf("Savefile '%s' not found!\n", path);
        return -1;
    } else
        printf("Saving updated progress to '%s'\n", path);

    /* Write header */
    fprintf(file, "# Super Mario 64 save file\n");
    fprintf(file, "# Comment starts with #\n");
    fprintf(file, "# True = 1, False = 0\n");

    /* Write current timestamp */
    get_timestamp(value);
    fprintf(file, "# %s\n", value);

    /* Write MainMenuSaveData info */
    menudata = &gSaveBuffer.menuData[0];
    fprintf(file, "\n[menu]\n");
    fprintf(file, "coin_score_age = %d\n", menudata->coinScoreAges[fileIndex]);
    
    /* Sound mode */
    if (menudata->soundMode == 0) {
        fprintf(file, "sound_mode = %s\n", sound_modes[0]);     // stereo
    }
    else if (menudata->soundMode == 3) {
        fprintf(file, "sound_mode = %s\n", sound_modes[1]);     // mono
    }
    else if (menudata->soundMode == 1) {
        fprintf(file, "sound_mode = %s\n", sound_modes[2]);     // headset
    }
    else {
        printf("Undefined sound mode!");
        return -1;
    }
    
    /* Write all flags */
    fprintf(file, "\n[flags]\n");
    for (i = 1; i < NUM_FLAGS; i++) {
        if (strcmp(sav_flags[i], "")) {
            flags = save_file_get_flags();
            flags = (flags & (1 << i));      /* Get a specific bit */
            flags = (flags) ? 1 : 0;         /* Determine if bit is set or not */

            fprintf(file, "%s = %d\n", sav_flags[i], flags);
        }
    }

    /* Write coin count and star flags from each course (except Castle Grounds) */
    fprintf(file, "\n[courses]\n");
    for (i = 0; i < NUM_COURSES; i++) {
        stars = save_file_get_star_flags(fileIndex, i);
        coins = save_file_get_course_coin_score(fileIndex, i);
        starFlags = int_to_bin(stars);          /* 63 -> 111111 */
            
        fprintf(file, "%s = \"%d, %07d\"\n", sav_courses[i], coins, starFlags);
    }

    /* Write star flags from each bonus cource (including Castle Grounds) */
    fprintf(file, "\n[bonus]\n");
    for (i = 0; i < NUM_BONUS_COURSES; i++) {
        char *format;

        if (i == NUM_BONUS_COURSES-1) {
            /* Process Castle Grounds */
            stars = save_file_get_star_flags(fileIndex, -1);
            format = "%05d";
        } else if (i == 3) {
            /* Process Princess's Secret Slide */
            stars = save_file_get_star_flags(fileIndex, 18);
            format = "%02d";
        } else {
            /* Process another bonus course */
            stars = save_file_get_star_flags(fileIndex, i+15);
            format = "%d";
        }

        starFlags = int_to_bin(stars);
        if (sprintf(value, format, starFlags) < 0)
            return -1;
        fprintf(file, "%s = %s\n", sav_bonus_courses[i], value);
    }

    /* Write who steal Mario's cap */
    for (i = 0; i < NUM_CAP_ON; i++) {
        flags = save_file_get_flags();      // Read all flags
        bit = (1 << (i+16));                // Determine current flag
        flags = (flags & bit);              // Get `cap` flag
        flags = (flags) ? 1 : 0;            // Determine if bit is set or not
        if (flags) {
            fprintf(file, "\n[cap]\n");
            fprintf(file, "type = %s\n", cap_on_types[i]);
            break;
        }
    }

    /* Write in what course and area Mario losted its cap, and cap's position */
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

    /* Update a backup */
    bcopy(&gSaveBuffer.files[fileIndex][0], &gSaveBuffer.files[fileIndex][1],
          sizeof(gSaveBuffer.files[fileIndex][1]));
    
    fclose(file);
    return 1;
}

/**
 * Read gSaveBuffer data from a text-based savefile.
 */
static s32 read_text_save(s32 fileIndex) {
    char path[SYS_MAX_PATH] = { 0 };
    char temp[32] = { 0 };
    const char *value;
    ini_t *savedata;
    
    u32 i, flag, coins, stars, starFlags;
    u32 capArea;
    
    /* Load savefile from defined path */
    snprintf(path, sizeof(path), "%s\\sm64_save_file_%d.sav", sys_save_path(), fileIndex);

    /* Try to open the file */
    savedata = ini_load(path);
    if (savedata == NULL) {
        printf("Savefile '%s' not found!\n", path);
        return -1;
    } else {
        printf("Loading savefile from '%s'\n", path);
    }

    /* Read coin score age for selected file and sound mode */
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
    
    /* Parse main flags */
    for (i = 1; i < NUM_FLAGS; i++) {
        value = ini_get(savedata, "flags", sav_flags[i]);
        
        if (value) {
            flag = strtol(value, &temp, 10);
            if (flag) {
                flag = 1 << i;  /* Look #define in header.. */
                gSaveBuffer.files[fileIndex][0].flags |= flag;
            }
        }
    }

    /* Parse coin and star values for each main course */
    for (i = 0; i < NUM_COURSES; i++) {
        value = ini_get(savedata, "courses", sav_courses[i]);
        if (value) {
            sscanf(value, "%d, %d", &coins, &stars); 
            starFlags = bin_to_int(stars);  /* 111111 -> 63 */

            save_file_set_star_flags(fileIndex, i, starFlags);
            gSaveBuffer.files[fileIndex][0].courseCoinScores[i] = coins;
        }
    }

    /* Parse star values for each bonus course */
    for (i = 0; i < NUM_BONUS_COURSES; i++) {
        value = ini_get(savedata, "bonus", sav_bonus_courses[i]);
        if (value) {
            sscanf(value, "%d", &stars);
            starFlags = bin_to_int(stars);

            if (strlen(value) == 5) {
                /* Process Castle Grounds */
                save_file_set_star_flags(fileIndex, -1, starFlags);
            } else if (strlen(value) == 2) {
                /* Process Princess's Secret Slide */
                save_file_set_star_flags(fileIndex, 18, starFlags);
            } else {
                /* Process another shitty bonus course */
                save_file_set_star_flags(fileIndex, i+15, starFlags);
            }
        }
    }

    /* Find, who steal Mario's cap ... */
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
    
    /* ... it's level ... */
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
    
    /* ... and it's area */
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
    
    /* Good, file exists for gSaveBuffer */
    gSaveBuffer.files[fileIndex][0].flags |= SAVE_FLAG_FILE_EXISTS;

    /* Make a backup */
    bcopy(&gSaveBuffer.files[fileIndex][0], &gSaveBuffer.files[fileIndex][1],
          sizeof(gSaveBuffer.files[fileIndex][1]));
    
    /* Cleaning up after ourselves */
    ini_free(savedata);
    return 0;
}
