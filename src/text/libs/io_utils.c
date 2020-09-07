#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include "io_utils.h"

#define _READFILE_GUESS 256

void combine(char* destination, const char* path1, const char* path2) {
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
        if(append_directory_separator)
            strcat(destination, directory_separator);
        strcat(destination, path2);
    }
}

const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

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