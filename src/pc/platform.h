#ifndef _SM64_PLATFORM_H_
#define _SM64_PLATFORM_H_

#include <stdbool.h>

/* Platform-specific functions and whatnot */

#define DATADIR "res"
#define SYS_MAX_PATH 1024 // FIXME: define this on different platforms

bool sys_mkdir(const char *name); // creates with 0777 by default
const char *sys_data_path(void);
const char *sys_save_path(void);
const char *sys_exe_path(void);

#endif // _SM64_PLATFORM_H_
