#include <string.h>

#define FILENAME_FORMAT "save_file_%d.sav"
#define NUM_COURSES 15
#define NUM_BONUS_COURSES 10
#define NUM_FLAGS 21
#define NUM_CAP_ON 4

/* Flag key */
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
    "hub", "bitdw", "bitfs", "bits", "pss", "cotmc",
    "totwc", "vcutm", "wmotr", "sa",
};

/* Mario's cap type keys */
const char *cap_on_types[NUM_CAP_ON] = {
    "ground", "klepto", "ukiki", "mrblizzard"
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
    char filename[32] = { 0 };
    char value[32] = { 0 };
    u32 i, flags, coins, stars, starFlags;

    /* Define savefile's name */
    if (sprintf(filename, FILENAME_FORMAT, fileIndex) < 0)
        return -1;

    file = fopen(filename, "wt");
    if (file == NULL)
        return -1;

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
    fprintf(file, "sound_mode = %u\n", menudata->soundMode);

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
        if (i == 0) {
            stars = save_file_get_star_flags(fileIndex, -1);
        } else {
            stars = save_file_get_star_flags(fileIndex, i+15);
        }
        starFlags = int_to_bin(stars);

        fprintf(file, "%s = %d\n", sav_bonus_courses[i], starFlags);
    }

    /* Write who steal Mario's cap */
    fprintf(file, "\n[cap]\n");
    for (i = 0; i < NUM_CAP_ON; i++) {
        flags = save_file_get_flags();      // Read all flags
        flags = (flags & (1 << (i+16)));    // Get `cap` flags
        flags = (flags) ? 1 : 0;            // Determine if bit is set or not
        if (flags) {
            fprintf(file, "type = %s\n", cap_on_types[i]);
            break;
        }
    }

    /* Write in what course and area Mario losted its cap, and cap's position */
    savedata = &gSaveBuffer.files[fileIndex][0];
    fprintf(file, "level = %d\n", savedata->capLevel);
    fprintf(file, "area = %d\n", savedata->capArea);

    fclose(file);
    return 1;
}

/**
 * Read gSaveBuffer data from a text-based savefile.
 */
static s32 read_text_save(s32 fileIndex) {
    char filename[32] = { 0 };
    char temp[32] = { 0 };
    const char *value;
    ini_t *savedata;
    
    u32 i, flag, coins, stars, starFlags;
    u32 capLevel, capArea;
    Vec3s capPos;
    
    /* Define savefile's name */
    if (sprintf(filename, FILENAME_FORMAT, fileIndex) < 0)
        return -1;

    /* Try to open the file */
    savedata = ini_load(filename);
    if (savedata == NULL) {
        return -1;
    } else {
        /* Good, file exists for gSaveBuffer */
        gSaveBuffer.files[fileIndex][0].flags |= SAVE_FLAG_FILE_EXISTS;
    }

    /* Read coin score age for selected file and sound mode */
    ini_sget(savedata, "menu", "coin_score_age", "%d",
                &gSaveBuffer.menuData[0].coinScoreAges[fileIndex]);
    ini_sget(savedata, "menu", "sound_mode", "%u",
                &gSaveBuffer.menuData[0].soundMode);    // Can override 4 times! 
    
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
                save_file_set_star_flags(fileIndex, COURSE_PSS, starFlags);
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
    
    /* ... also it's position, area and level */
    sscanf(ini_get(savedata, "cap", "level"), "%d", &capLevel);
    sscanf(ini_get(savedata, "cap", "area"), "%d", &capArea);
    
    gSaveBuffer.files[fileIndex][0].capLevel = capLevel; 
    gSaveBuffer.files[fileIndex][0].capArea = capArea; 

    /* Cleaning up after ourselves */
    ini_free(savedata);
    return 1;
}
