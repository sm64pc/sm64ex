#include "text-loader.h"
#include "txtconv.h"
#include <stdio.h>
#include <string.h>
#include "dialog_ids.h"
#include <limits.h>
#include "libs/io_utils.h"
#include <stdlib.h>
#include <dirent.h>
#include "libs/cJSON.h"
#include "pc/configfile.h"
#include "pc/fs/fs.h"

#define MAX_LANG 30
#define SECRET_NULL 24

struct DialogEntry * *dialogPool;
u8 * *seg2_course_name_table;
u8 * *seg2_act_name_table;

struct LanguageEntry * *languages;
u8 languagesAmount = 0;

struct LanguageEntry *current_language;

void load_language(char *jsonTxt, s8 language) {
    languages[language] = malloc (sizeof (struct LanguageEntry));

    const char *endTxt;
    cJSON *json = cJSON_ParseWithOpts(jsonTxt, &endTxt, 1);

    if(*endTxt != 0) {
        fprintf(stderr, "Loading File: Error before: %s\n", cJSON_GetErrorPtr());
        exit(1);
    }

    const cJSON *manifest = cJSON_GetObjectItemCaseSensitive(json, "manifest");
    const cJSON *dialogs = cJSON_GetObjectItemCaseSensitive(json, "dialogs");
    const cJSON *courses = cJSON_GetObjectItemCaseSensitive(json, "courses");
    const cJSON *secrets = cJSON_GetObjectItemCaseSensitive(json, "secrets");
    const cJSON *options = cJSON_GetObjectItemCaseSensitive(json, "options");
    const cJSON *strings = cJSON_GetObjectItemCaseSensitive(json, "strings");
    const cJSON *dialog = NULL;
    const cJSON *course = NULL;
    const cJSON *secret = NULL;

    char *nametmp = cJSON_GetObjectItemCaseSensitive(manifest, "langName")->valuestring;
    char *logotmp = cJSON_GetObjectItemCaseSensitive(manifest, "langLogo")->valuestring;

    char *name = malloc(strlen(nametmp) + 1);
    char *logo = malloc(strlen(logotmp) + 1);

    strcpy(name, nametmp);
    strcpy(logo, logotmp);

    languages[language]->name = name;
    languages[language]->logo = logo;

    languages[language]->dialogs = malloc(DIALOG_COUNT * sizeof(struct DialogEntry));

    int eid = 0;
    cJSON_ArrayForEach(dialog, dialogs) {
        int id = cJSON_GetObjectItemCaseSensitive(dialog, "ID")->valueint;

        struct DialogEntry *entry = malloc(sizeof(struct DialogEntry));

        entry->unused = 1;
        entry->linesPerBox = cJSON_GetObjectItemCaseSensitive(dialog, "linesPerBox")->valueint;
        entry->leftOffset = cJSON_GetObjectItemCaseSensitive(dialog, "leftOffset")->valueint;
        entry->width = cJSON_GetObjectItemCaseSensitive(dialog, "width")->valueint;

        const cJSON *line = NULL;
        const cJSON *lines = NULL;
        lines = cJSON_GetObjectItemCaseSensitive(dialog, "lines");

        int lineAmount = cJSON_GetArraySize(lines);
        int dialogSize = lineAmount * 45 * 7;
        char *dialogTxt = malloc(dialogSize * sizeof(char));
        strcpy(dialogTxt, "");
        int currLine = 0;
        cJSON_ArrayForEach(line, lines) {
            char *str = line->valuestring;
            strcat(dialogTxt, str);
            if(currLine < lineAmount - 1) {
                strcat(dialogTxt, "\n");
                currLine++;
            }
        }

        entry->str = getTranslatedText(dialogTxt);
        languages[language]->dialogs[eid] = entry;
        eid++;
        free(dialogTxt);
    }

    int course_name_table_size = cJSON_GetArraySize(courses) + cJSON_GetArraySize(secrets);

    int MAXLENGTH = 40;

    languages[language]->courses = malloc((sizeof(char*) * MAXLENGTH) * course_name_table_size);
    languages[language]->acts = malloc((sizeof(char*) * MAXLENGTH) * 255);
    int courseID = 0;
    int actID = 0;

    int actAmount = cJSON_GetArraySize(courses);
    int actSize = 0;

    cJSON_ArrayForEach(course, courses) {
        const cJSON *acts = cJSON_GetObjectItemCaseSensitive(course, "acts");
        const cJSON *act = NULL;
        char *courseName = cJSON_GetObjectItemCaseSensitive(course, "course")->valuestring;

        if(courseID + 1 <= cJSON_GetArraySize(courses) - 1) {
            languages[language]->courses[courseID] = getTranslatedText(courseName);
            courseID++;
        }

        cJSON_ArrayForEach(act, acts) {
            languages[language]->acts[actID] = getTranslatedText(act->valuestring);
            actSize += strlen(act->valuestring);
            actID++;
        }

        actAmount += cJSON_GetArraySize(acts);
        actSize += strlen(courseName);
    }

    languages[language]->acts = realloc(languages[language]->acts, sizeof(u8*) * (actAmount * actSize));

    int padding = 0;

    cJSON_ArrayForEach(secret, secrets) {
        if((courseID == SECRET_NULL) && (secret->valuestring != NULL)) {
            languages[language]->courses[courseID] = getTranslatedText(0);
            padding++;
        }

        languages[language]->courses[courseID + padding] = getTranslatedText(secret->valuestring);
        courseID++;
    }

    size_t stringSize = cJSON_GetArraySize(options) + cJSON_GetArraySize(strings);

    languages[language]->num_strings = stringSize;
    languages[language]->strings = malloc(sizeof(struct StringTable) * stringSize);

    int stringID = 0;
    cJSON *option;
    cJSON *str;

    cJSON_ArrayForEach(option, options) {
        struct StringTable *entry = malloc (sizeof(struct StringTable));

        char* key = malloc(strlen(option->string) + 1);
        char* value = malloc(strlen(option->valuestring) + 1);

        strcpy(key, option->string);
        strcpy(value, option->valuestring);

        entry->key = key;
        entry->value = getTranslatedText(value);
        languages[language]->strings[stringID] = entry;
        free(value);
        stringID++;
    }

    cJSON_ArrayForEach(str, strings) {
        struct StringTable *entry = malloc (sizeof (struct StringTable));

        char* key = malloc(strlen(str->string) + 1);
        char* value = malloc(strlen(str->valuestring) + 1);

        strcpy(key, str->string);
        strcpy(value, str->valuestring);

        entry->key = key;
        entry->value = getTranslatedText(value);
        languages[language]->strings[stringID] = entry;
        free(value);
        stringID++;
    }

    cJSON_Delete(json);
}

struct LanguageEntry *get_language_by_name(char *name) {
    int id = 0;

    for(int l = 0; l < languagesAmount; l++) {
        if(strcmp(languages[l]->name, name) == 0) {
            id = l;
            break;
        }
    }

    return languages[id];
}

struct LanguageEntry *get_language() {
    return current_language;
}

void set_language(struct LanguageEntry *new_language) {
    current_language = new_language;
    dialogPool = new_language->dialogs;
    seg2_act_name_table = new_language->acts;
    seg2_course_name_table = new_language->courses;
}

u8 *get_key_string(char *id) {
    struct LanguageEntry *current = current_language;

    u8 *tmp = getTranslatedText("NONE");

    for(int stringID = 0; stringID < current->num_strings; stringID++) {
        struct StringTable *str = current->strings[stringID];
        if(strcmp(str->key, id) == 0) {
            free(tmp);
            tmp = str->value;
            break;
        }
    }

    return tmp;
}

static bool alloc_language(void *user, const char *path) {
    const char *extension = get_filename_ext(path);
    if(strcmp(extension, "json") == 0) {        
        languagesAmount++;
        
        fprintf(stderr, "Loading File '%s'\n", path);

        u64 lngSize = 0;
        char *jsonTxt = fs_load_file(path, &lngSize);
        jsonTxt[lngSize] = '\0';
        
        if(jsonTxt != NULL) {
            load_language(jsonTxt, languagesAmount - 1);
        }else{
            fprintf(stderr, "Loading File: Error reading '%s'\n", path);
            exit(1);
        }
        free(jsonTxt);        
    }
    return true;
}

void alloc_dialog_pool(char *exePath, char *gamedir) {
    languages = malloc(sizeof(struct LanguageEntry*) * 50);

    fs_walk("texts", alloc_language, NULL, true);
    
     if(languagesAmount > 0) {
        languages = realloc(languages, sizeof(struct LanguageEntry*) * (languagesAmount));
    }else{
        fprintf(stderr, "Loading File: No language files found, aborting.\n");
        exit(1);
    }

    if(configLanguage >= languagesAmount) {
        printf("Loading File: Configured language doesn't exist, resetting to defaults.\n");
        configLanguage = 0;
    }
    set_language(languages[configLanguage]);
}

void dealloc_dialog_pool(void) {
    for(int l = 0; l < languagesAmount; l++) {
        struct LanguageEntry * entry = languages[l];
        for(int i = 0; i < entry->num_strings; i++) free(entry->strings[i]);
        for(int i = 0; i < sizeof(entry->acts) / sizeof(entry->acts[0]); i++) free(entry->acts[i]);
        for(int i = 0; i < sizeof(entry->courses) / sizeof(entry->courses[0]); i++) free(entry->courses[i]);
        for(int i = 0; i < DIALOG_COUNT; i++) free(entry->dialogs[i]);
        free(entry->strings);
        free(entry->acts);
        free(entry->courses);
        free(entry->dialogs);
        free(entry->logo);
        free(entry->name);
        free(languages[l]); 
    }

    free(languages);
}
