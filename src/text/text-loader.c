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

#define PLACEHOLDER "You are not supposed\nto be here.\n\nKeep this as a secret\n\n- Render96 Team"

struct DialogEntry* * dialogPool;
u8* * seg2_course_name_table;
u8* * seg2_act_name_table;

struct LanguageEntry* * languages;
s8 languagesAmount = 0;

struct LanguageEntry* current_language;

void load_language(char* jsonTxt, s8 language){    
    languages[language] = malloc (sizeof (struct LanguageEntry));

    cJSON *json = cJSON_Parse(jsonTxt);

    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();

        if (error_ptr != NULL) {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        } else {
            fprintf(stderr, "Error loading the JSON file\n");
        }
        exit(1);
    }
    
    const cJSON *dialogs = cJSON_GetObjectItemCaseSensitive(json, "dialogs");
    const cJSON *courses = cJSON_GetObjectItemCaseSensitive(json, "courses");
    const cJSON *secrets = cJSON_GetObjectItemCaseSensitive(json, "secrets");
    const cJSON *options = cJSON_GetObjectItemCaseSensitive(json, "options");
    const cJSON *strings = cJSON_GetObjectItemCaseSensitive(json, "strings");
    const cJSON *manifest = cJSON_GetObjectItemCaseSensitive(json, "manifest");
    const cJSON *dialog = NULL;
    const cJSON *course = NULL;    
    const cJSON *secret = NULL;        

    languages[language]->dialogs = malloc(DIALOG_COUNT * sizeof(struct DialogEntry));

    languages[language]->name = cJSON_GetObjectItemCaseSensitive(manifest, "langName")->valuestring;
    languages[language]->logo = cJSON_GetObjectItemCaseSensitive(manifest, "langLogo")->valuestring;    

    int eid = 0;
    cJSON_ArrayForEach(dialog, dialogs) {
        int id = cJSON_GetObjectItemCaseSensitive(dialog, "ID")->valueint;        

        struct DialogEntry *entry = malloc (sizeof (struct DialogEntry));

        entry->unused = 1;
        entry->linesPerBox = cJSON_GetObjectItemCaseSensitive(dialog, "linesPerBox")->valueint;
        entry->leftOffset = cJSON_GetObjectItemCaseSensitive(dialog, "leftOffset")->valueint;
        entry->width = cJSON_GetObjectItemCaseSensitive(dialog, "width")->valueint;

        const cJSON *line = NULL;
        const cJSON *lines = NULL;
        lines = cJSON_GetObjectItemCaseSensitive(dialog, "lines");

        int lineAmount = cJSON_GetArraySize(lines);
        int dialogSize = lineAmount * 45;
        char* dialogTxt = malloc(dialogSize * sizeof(char));
        strcpy(dialogTxt, "");
        int currLine = 0;
        cJSON_ArrayForEach(line, lines) {
            char * str = line->valuestring;
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
        char* courseName = cJSON_GetObjectItemCaseSensitive(course, "course")->valuestring;

        languages[language]->courses[courseID] = getTranslatedText(courseName);
        courseID++;
        cJSON_ArrayForEach(act, acts) {
            languages[language]->acts[actID] = getTranslatedText(act->valuestring);
            actSize += strlen(act->valuestring);
            actID++;
        }
        actAmount += cJSON_GetArraySize(acts);
        actSize += strlen(courseName);
    }

    languages[language]->acts = realloc(languages[language]->acts, sizeof(u8*) * (actAmount * actSize));

    cJSON_ArrayForEach(secret, secrets) {
        languages[language]->courses[courseID] = getTranslatedText(secret->valuestring);
        courseID++;
    }
    
    size_t stringSize = cJSON_GetArraySize(options) + cJSON_GetArraySize(strings);

    languages[language]->string_length = stringSize;
    languages[language]->strings = malloc(sizeof(struct StringTable) * stringSize);

    int stringID = 0;
    cJSON* option;
    cJSON* str;

    cJSON_ArrayForEach(option, options) {        
        struct StringTable *entry = malloc (sizeof (struct StringTable));        

        char* key = malloc(strlen(option->string) + 1);
        char* value = malloc(strlen(option->valuestring) + 1);

        strcpy(key, option->string);
        strcpy(value, option->valuestring);

        entry->key = key;
        entry->value = getTranslatedText(value);
        languages[language]->strings[stringID] = entry;
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
        stringID++;
    }
    
    cJSON_free(json);
}

void alloc_languages(char* exePath, char* gamedir){
    languages = realloc(languages, sizeof(struct LanguageEntry*) * 30);

    char *lastSlash = NULL;
    char *parent = malloc(FILENAME_MAX * sizeof(char*));
    #ifndef WIN32
    lastSlash = strrchr(exePath, '/');
    #else
    lastSlash = strrchr(exePath, '\\');
    #endif
    strncpy(parent, exePath, strlen(exePath) - strlen(lastSlash));

    char * languagesDir = malloc(FILENAME_MAX * sizeof(char*));
    strcpy(languagesDir, parent);
    strcat(languagesDir, "/");
    strcat(languagesDir, gamedir);
    strcat(languagesDir, "/texts/");

    DIR *lf = opendir(languagesDir);
    struct dirent *de;
    while ((de = readdir(lf)) != NULL){
        const char* extension = get_filename_ext(de->d_name);
        char * file = malloc(FILENAME_MAX * sizeof(char*));
        if(strcmp(extension, "json") == 0){                                    
            
            strcpy(file, languagesDir);
            strcat(file, de->d_name);
            languagesAmount++;
            printf("Loading File: %s\n", file);

            char * jsonTxt = read_file(file);
            load_language(jsonTxt, languagesAmount - 1);
            free(file);
        }        
    }

    free(languagesDir);

    languages = realloc(languages, sizeof(struct LanguageEntry*) * (languagesAmount));
}

struct LanguageEntry* get_language_by_name(char* name){
    int id = 0;

    for(int l = 0; l < languagesAmount; l++){
        if(strcmp(languages[l]->name, name) == 0){
            id = l;            
            break;
        }
   }

   return languages[id];
}

struct LanguageEntry* get_language(){
   return current_language;
}

void set_language(struct LanguageEntry* new_language){    
    current_language = new_language;
    printf("SWITCHED: %s\n", current_language->name);
    dialogPool = new_language->dialogs;
    seg2_act_name_table = new_language->acts;
    seg2_course_name_table = new_language->courses;
}

u8* get_key_string(char* id){
    struct LanguageEntry * current = current_language;    

    u8* tmp = getTranslatedText("NONE");
    
    for(int stringID = 0; stringID < current->string_length; stringID++){
        struct StringTable * str = current->strings[stringID];                
        if(strcmp(str->key, id) == 0){
            tmp = str->value;
            break;
        }
    }
    
    return tmp;
}

void alloc_dialog_pool(char* exePath, char* gamedir){
    languages = malloc(sizeof(struct LanguageEntry*));
    
    languages[0] = malloc (sizeof (struct LanguageEntry));
    languages[0]->name = "none";
    languages[0]->logo = "none";    
    languages[0]->dialogs = malloc(DIALOG_COUNT * sizeof(struct DialogEntry));
    
    for(int i = 0; i < DIALOG_COUNT; i++){
        struct DialogEntry *entry = malloc (sizeof (struct DialogEntry));

        entry->unused = 1;
        entry->linesPerBox = 6;
        entry->leftOffset = 95;
        entry->width = 200;
        entry->str = getTranslatedText(PLACEHOLDER);

        languages[0]->dialogs[i] = entry;
    }    

    alloc_languages(exePath, gamedir);
    set_language(languages[configLanguage]);
}

void dealloc_dialog_pool(void){
    for(int l = 0; l < languagesAmount; l++){
        struct LanguageEntry * entry = languages[l];
        for(int i = 0; i < sizeof(entry->acts) / sizeof(entry->acts[0]); i++) free(entry->acts[i]);
        for(int i = 0; i < sizeof(entry->courses) / sizeof(entry->courses[0]); i++) free(entry->courses[i]);
        for(int i = 0; i < DIALOG_COUNT; i++) free(entry->dialogs[i]);
        for(int i = 0; i < entry->string_length; i++) free(entry->strings[i]);
        free(entry->acts);
        free(entry->courses);
        free(entry->dialogs);
        free(entry->logo);
        free(entry->name);
        free(entry->strings);
        entry->acts = NULL;
        entry->courses = NULL;
        entry->dialogs = NULL;
        entry->logo = NULL;
        entry->name = NULL;
        entry->strings = NULL;
    }

    free(languages);
}