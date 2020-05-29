#ifndef _SM64_PLATFORM_H_
#define _SM64_PLATFORM_H_

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* Platform-specific functions and whatnot */

#define DATADIR "res"
#define SYS_MAX_PATH 1024 // FIXME: define this on different platforms

// crossplatform impls of misc stuff
char *sys_strdup(const char *src);
char *sys_strlwr(char *src);
int sys_strcasecmp(const char *s1, const char *s2);

// filesystem stuff
bool sys_mkdir(const char *name); // creates with 0777 by default
bool sys_file_exists(const char *name);
bool sys_dir_exists(const char *name);

// receives the full path
// should return `true` if traversal should continue
typedef bool (*walk_fn_t)(const char *);
// returns `true` if the directory was successfully opened and walk() didn't ever return false
bool sys_dir_walk(const char *base, walk_fn_t walk, const bool recur);

// path stuff
const char *sys_data_path(void);
const char *sys_save_path(void);
const char *sys_exe_path(void);

#endif // _SM64_PLATFORM_H_
