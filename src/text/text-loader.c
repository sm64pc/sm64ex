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

struct DialogEntry* * dialogPool;
struct LanguageEntry* * languages;
s8 languagesAmount = 0;

char* currentLanguage = "english";

void preloadLanguageText(char* jsonTxt, s8 language){    
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

    const cJSON *dialog = NULL;
    const cJSON *dialogs = NULL;
    const cJSON *manifest = cJSON_GetObjectItemCaseSensitive(json, "manifest");

    struct DialogEntry** entries = malloc(DIALOG_COUNT * sizeof(struct DialogEntry));

    languages[language]->name = cJSON_GetObjectItemCaseSensitive(manifest, "languageName")->valuestring;
    languages[language]->logo = cJSON_GetObjectItemCaseSensitive(manifest, "languageLogo")->valuestring;
    languages[language]->placeholder = cJSON_GetObjectItemCaseSensitive(manifest, "placeholder")->valuestring;

    dialogs = cJSON_GetObjectItemCaseSensitive(json, "dialogs");

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
        entries[eid] = entry;
        eid++;
        free(dialogTxt);
    }
    
    languages[language]->entries = entries;

    cJSON_Delete(json);
}

void alloc_languages(void){
    languages = realloc(languages, sizeof(struct LanguageEntry*) * 30);

    char * languagesDir = "./res/texts/";
    DIR *lf = opendir(languagesDir);
    struct dirent *de;
    while ((de = readdir(lf)) != NULL){
        const char* extension = get_filename_ext(de->d_name);
        char * file = malloc(99 * sizeof(char*));
        if(strcmp(extension, "json") == 0){
            strcpy(file, "");
            strcat(file, "./res/texts/");
            strcat(file, de->d_name);

            #ifndef WIN32
            char * language_file = realpath(file, NULL);
            #else
            char * language_file = malloc(_MAX_PATH * sizeof(char));
            _fullpath(language_file, file, _MAX_PATH );
            #endif

            printf("Loading File: %s\n", language_file);
            languagesAmount++;

            char * jsonTxt = read_file(language_file);
            preloadLanguageText(jsonTxt, languagesAmount - 1);
        }
    }

    languages = realloc(languages, sizeof(struct LanguageEntry*) * (languagesAmount));
}

void selectLanguage(char* languageName){

    char* lowerName = malloc(sizeof(languageName));
    for(char l = 0; l < sizeof(lowerName) / sizeof(char); l++)
        lowerName[l] = tolower(languageName[l]);

    int id = 0;
    char* languageTmp = "none";

    for(int l = 0; l < languagesAmount; l++){
        if(strcmp(languages[l]->name, lowerName) == 0){
            id = l;
            languageTmp = languages[l]->name;
            break;
        }
    }

    currentLanguage = languageTmp;
    dialogPool = languages[id]->entries;
}

void alloc_dialog_pool(void){
    languages = malloc(sizeof(struct LanguageEntry*));
        
    languages[0] = malloc (sizeof (struct LanguageEntry));
    languages[0]->name = "none";
    languages[0]->logo = "none";
    languages[0]->placeholder = "You are not supposed\nto be here.\n\nKeep this as a secret\n\n- Render96 Team";
    languages[0]->entries = malloc(DIALOG_COUNT * sizeof(struct DialogEntry));
    
    for(int i = 0; i < DIALOG_COUNT; i++){
        struct DialogEntry *entry = malloc (sizeof (struct DialogEntry));

        entry->unused = 1;
        entry->linesPerBox = 6;
        entry->leftOffset = 95;
        entry->width = 200;
        entry->str = getTranslatedText(languages[0]->placeholder);

        languages[0]->entries[i] = entry;
    }    

    alloc_languages();
    selectLanguage(currentLanguage);
}
