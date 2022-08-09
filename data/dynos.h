#ifndef DYNOS_H
#define DYNOS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>
#include <dirent.h>
#include <SDL2/SDL.h>
#ifdef __cplusplus
#include <new>
#include <utility>
#include <string>
extern "C" {
#endif
#include "types.h"
#include "config.h"
#include "pc/fs/fs.h"
#include "audio_defines.h"
#include "engine/math_util.h"
#undef STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#ifdef __cplusplus
}
#endif

#define DYNOS_VERSION           "1.0"
#define DYNOS_EXE_FOLDER        sys_exe_path()
#define DYNOS_USER_FOLDER       sys_user_path()
#define DYNOS_RES_FOLDER        "dynos"
#define DYNOS_PACKS_FOLDER      DYNOS_RES_FOLDER "/packs"
#define DYNOS_CONFIG_FILENAME   "DynOS." DYNOS_VERSION ".config.txt"
#define DYNOS_AT_STARTUP		__attribute__((constructor))
#define DYNOS_AT_EXIT			__attribute__((destructor))

#endif