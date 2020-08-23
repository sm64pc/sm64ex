#include "text-loader.h"
#include "txtconv.h"
#include "libs/cJSON.h"
#include <stdio.h>
#include <string.h>
#include "dialog_ids.h"
#include "game/ingame_menu.h"
#include <unistd.h>
#include <limits.h>
#include "libs/dir_utils.h"

struct DialogEntry* * dialogPool;

char* load_file(char const* path) {
    char* buffer = 0;
    long length = 0;
    FILE * f = fopen (path, "rb");        

    if (f) {
      fseek (f, 0, SEEK_END);
      length = ftell (f);
      fseek (f, 0, SEEK_SET);
      buffer = (char*) malloc((length+1)*sizeof(char));
      if (buffer) {
        fread(buffer, sizeof(char), length, f);
      }
      fclose(f);
      buffer[length] = '\0';
      return buffer;
    }
    return NULL;
}

void preloadTexts(){

    char * cwd = "/home/alex/Documents/Projects/Render96/Render96ex - FastBuild/build/us_pc";
    //getcwd(cwd, sizeof(cwd));    
    #ifndef WIN32
    char * file = "/res/texts/es.json";
    #else
    char * file = "\\res\\texts\\es.json";
    #endif

    char * language_file = malloc((strlen(cwd) + strlen(file)) * sizeof(char));
    strcpy(language_file, "");
    strcat(language_file, cwd);
    strcat(language_file, file);

    printf("Loading File: %s\n", language_file);

    char * jsonTxt = load_file( language_file );
    if(jsonTxt == NULL) return;

    cJSON *json = cJSON_Parse(jsonTxt);
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error before: %s\n", error_ptr);
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
    free(jsonTxt);
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