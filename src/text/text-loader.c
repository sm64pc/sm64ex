#include "text-loader.h"
#include "txtconv.h"
#include "libs/cJSON.h"
#include <stdio.h>
#include <string.h>
#include "dialog_ids.h"
#include "game/ingame_menu.h"
#include <limits.h>
#include "libs/dir_utils.h"
#include <stdlib.h>
struct DialogEntry* * dialogPool;
#define _READFILE_GUESS 256

#define SPANISH "./res/texts/es.json"
#define ENGLISH "./res/texts/us.json"

char* read_file(char* name){
    FILE* file;
    file = fopen(name, "r");

    if(!file)
        return NULL;

    char* result = malloc(sizeof(char) * _READFILE_GUESS + 1);

    if(result == NULL)
        return NULL;

    size_t pos = 0;
    size_t capacity = _READFILE_GUESS;
    char ch;

    while((ch = getc(file)) != EOF){
        result[pos++] = ch;

        if(pos >= capacity){
            capacity += _READFILE_GUESS;
            result = realloc(result, sizeof(char) * capacity + 1);
            if(result == NULL)
                return NULL;
        }
    }
    fclose(file);
    result = realloc(result, sizeof(char) * pos);
    if(result == NULL)
        return NULL;
    result[pos] = '\0';

    return result;
}

void preloadTexts(){

    char * file = ENGLISH;

    #ifndef WIN32
    char * language_file = realpath(file, NULL);
    #else
    char * language_file = malloc(_MAX_PATH * sizeof(char));
    _fullpath(language_file, file, _MAX_PATH );
    #endif

    printf("Loading File: %s\n", language_file);

    char * jsonTxt = read_file(language_file);

    cJSON *json = cJSON_Parse(jsonTxt);
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();

        if (error_ptr != NULL) {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }else{
            fprintf(stderr, "Error loading the JSON file\n");
        }
        exit(1);
    }
    const cJSON *dialog = NULL;
    const cJSON *dialogs = NULL;

    dialogs = cJSON_GetObjectItemCaseSensitive(json, "dialogs");

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
        dialogPool[id] = entry;
        free(dialogTxt);
    }

    cJSON_Delete(json);
}

void alloc_dialog_pool(void){
    dialogPool = malloc(DIALOG_COUNT * sizeof(struct DialogEntry));

    for(int i = 0; i < DIALOG_COUNT; i++){
        struct DialogEntry *entry = malloc (sizeof (struct DialogEntry));

        entry->unused = 1;
        entry->linesPerBox = 6;
        entry->leftOffset = 95;
        entry->width = 200;
        entry->str = getTranslatedText("You are not supposed\nto be here.\n\nKeep this as a secret\n\n- Render96 Team");
        dialogPool[i] = entry;
    }

    preloadTexts();
}
