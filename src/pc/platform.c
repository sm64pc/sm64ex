#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#ifdef _WIN32
#include <direct.h>
#endif

#include "cliopts.h"

/* these are not available on some platforms, so might as well */

char *sys_strlwr(char *src) {
  for (unsigned char *p = (unsigned char *)src; *p; p++)
     *p = tolower(*p);
  return src;
}

char *sys_strdup(const char *src) {
    const unsigned len = strlen(src) + 1;
    char *newstr = malloc(len);
    if (newstr) memcpy(newstr, src, len);
    return newstr;
}

int sys_strcasecmp(const char *s1, const char *s2) {
    const unsigned char *p1 = (const unsigned char *) s1;
    const unsigned char *p2 = (const unsigned char *) s2;
    int result;
    if (p1 == p2)
        return 0;
    while ((result = tolower(*p1) - tolower(*p2++)) == 0)
        if (*p1++ == '\0')
            break;
    return result;
}

/* file system stuff */

bool sys_file_exists(const char *name) {
    struct stat st;
    return (stat(name, &st) == 0 && S_ISREG(st.st_mode));
}

bool sys_dir_exists(const char *name) {
    struct stat st;
    return (stat(name, &st) == 0 && S_ISDIR(st.st_mode));
}

bool sys_dir_walk(const char *base, walk_fn_t walk, const bool recur) {
    char fullpath[SYS_MAX_PATH];
    DIR *dir;
    struct dirent *ent;

    if (!(dir = opendir(base))) {
        fprintf(stderr, "sys_dir_walk(): could not open `%s`\n", base);
        return false;
    }

    bool ret = true;

    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_name[0] == 0 || ent->d_name[0] == '.') continue; // skip ./.. and hidden files
        snprintf(fullpath, sizeof(fullpath), "%s/%s", base, ent->d_name);
        if (sys_dir_exists(fullpath)) {
            if (recur) {
                if (!sys_dir_walk(fullpath, walk, recur)) {
                    ret = false;
                    break;
                }
            }
        } else {
            if (!walk(fullpath)) {
                ret = false;
                break;
            }
        }
    }

    closedir(dir);
    return ret;
}

void *sys_load_res(const char *name) {
    char path[SYS_MAX_PATH] = { 0 };
    snprintf(path, sizeof(path), "%s/%s", sys_data_path(), name);

    FILE *f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    void *buf = malloc(size);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    fread(buf, 1, size, f);
    fclose(f);

    return buf;
}

bool sys_mkdir(const char *name) {
    #ifdef _WIN32
    return _mkdir(name) == 0;
    #else
    return mkdir(name, 0777) == 0;
    #endif
}

#if USE_SDL

// we can just ask SDL for most of this shit if we have it
#include <SDL2/SDL.h>

const char *sys_data_path(void) {
    static char path[SYS_MAX_PATH] = { 0 };

    if (!path[0]) {
        // prefer the override, if it is set
        // "!" expands to executable path
        if (gCLIOpts.DataPath[0]) {
            if (gCLIOpts.DataPath[0] == '!')
                snprintf(path, sizeof(path), "%s%s", sys_exe_path(), gCLIOpts.DataPath + 1);
            else
                snprintf(path, sizeof(path), "%s", gCLIOpts.DataPath);
            if (sys_dir_exists(path)) return path;
            printf("Warning: Specified data path ('%s') doesn't exist\n", path);
        }

        // then the executable directory
        snprintf(path, sizeof(path), "%s/" DATADIR, sys_exe_path());
        if (sys_dir_exists(path)) return path;

        // then the save path
        snprintf(path, sizeof(path), "%s/" DATADIR, sys_save_path());
        if (sys_dir_exists(path)) return path;

        #if defined(__linux__) || defined(__unix__)
        // on Linux/BSD try some common paths for read-only data
        const char *try[] = {
            "/usr/local/share/sm64pc/" DATADIR,
            "/usr/share/sm64pc/" DATADIR,
            "/opt/sm64pc/" DATADIR,
        };
        for (unsigned i = 0; i < sizeof(try) / sizeof(try[0]); ++i) {
            if (sys_dir_exists(try[i])) {
                strcpy(path, try[i]);
                return path;
            }
        }
        #endif

        // hope for the best
        strcpy(path, "./" DATADIR);
    }

    return path;
}

const char *sys_save_path(void) {
    static char path[SYS_MAX_PATH] = { 0 };

    if (!path[0]) {
        // if the override is set, use that
        // "!" expands to executable path
        if (gCLIOpts.SavePath[0]) {
            if (gCLIOpts.SavePath[0] == '!')
                snprintf(path, sizeof(path), "%s%s", sys_exe_path(), gCLIOpts.SavePath + 1);
            else
                snprintf(path, sizeof(path), "%s", gCLIOpts.SavePath);
            if (!sys_dir_exists(path) && !sys_mkdir(path)) {
                printf("Warning: Specified save path ('%s') doesn't exist and can't be created\n", path);
                path[0] = 0; // doesn't exist and no write access
            }
        }

        // didn't work? get it from SDL
        if (!path[0]) {
            char *sdlpath = SDL_GetPrefPath("", "sm64pc");
            if (sdlpath) {
                const unsigned int len = strlen(sdlpath);
                strncpy(path, sdlpath, sizeof(path));
                path[sizeof(path)-1] = 0;
                SDL_free(sdlpath);
                if (path[len-1] == '/' || path[len-1] == '\\')
                    path[len-1] = 0; // strip the trailing separator
                if (!sys_dir_exists(path) && !sys_mkdir(path))
                    path[0] = 0;
            }
        }

        // if all else fails, just store near the EXE
        if (!path[0])
            strcpy(path, sys_exe_path());

        printf("Save path set to '%s'\n", path);
    }

    return path;
}

const char *sys_exe_path(void) {
    static char path[SYS_MAX_PATH] = { 0 };

    if (!path[0]) {
        char *sdlpath = SDL_GetBasePath();
        if (sdlpath) {
            // use the SDL path if it exists
            const unsigned int len = strlen(sdlpath);
            strncpy(path, sdlpath, sizeof(path));
            path[sizeof(path)-1] = 0;
            SDL_free(sdlpath);
            if (path[len-1] == '/' || path[len-1] == '\\')
                path[len-1] = 0; // strip the trailing separator
        } else {
            // hope for the best
            strcpy(path, ".");
        }
        printf("Executable path set to '%s'\n", path);
    }

    return path;
}

#else

#warning "You might want to implement these functions for your platform"

const char *sys_data_path(void) {
    return ".";
}

const char *sys_save_path(void) {
    return ".";
}

const char *sys_exe_path(void) {
    return ".";
}

#endif // platform switch
