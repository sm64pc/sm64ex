#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include "io_utils.h"

void combine(char *destination, const char *path1, const char *path2) {
    if(path1 == NULL || path2 == NULL) {
        strcpy(destination, "");
    }
    else if(path2 == NULL || strlen(path2) == 0) {
        strcpy(destination, path1);
    }
    else if(path1 == NULL || strlen(path1) == 0) {
        strcpy(destination, path2);
    } 
    else {
        char directory_separator[] = "/";
#ifdef WIN32
        directory_separator[0] = '\\';
#endif
        const char *last_char = path1;
        while(*last_char != '\0')
            last_char++;        
        int append_directory_separator = 0;
        if(strcmp(last_char, directory_separator) != 0) {
            append_directory_separator = 1;
        }
        strcpy(destination, path1);
        if(append_directory_separator) {
            strcat(destination, directory_separator);
        }
        strcat(destination, path2);
    }
}

const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) { return ""; }
    return dot + 1;
}

char *read_file(char *name){
    char *result = NULL;
    FILE *file = fopen(name, "r");
    if (file != NULL) {
        // Go to end of ile
        if (fseek(file, 0L, SEEK_END) == 0) {
            
            // get size of file
            long bufsize = ftell(file);
            if (bufsize == -1) { return NULL; }

            // allocate buzzer to size
            result = malloc(sizeof(char) * (bufsize + 1));

            // go back to start of file
            if (fseek(file, 0L, SEEK_SET) != 0) { return NULL; }

            // read file into memory
            size_t newLen = fread(result, sizeof(char), bufsize, file);
            if ( ferror( file ) != 0 ) { return NULL; }
            else {
                result[newLen++] = '\0'; // just to be safe
            }
        }
        fclose(file);
    }
    return result;
}
