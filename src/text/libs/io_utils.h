#ifndef DIRUTILS
#define DIRUTILS

extern void combine(char* destination, const char* path1, const char* path2);
extern const char *get_filename_ext(const char *filename);
extern char* read_file(char* name);
#endif