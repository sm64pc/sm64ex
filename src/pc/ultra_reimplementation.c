#include <stdio.h>
#include <string.h>
#include "lib/src/libultra_internal.h"
#include "macros.h"

// These five next libraries are required for saving in the correct folders.
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <SDL2/SDL.h>
#include <errno.h>

u8 DONT_POLLUTE_SAVE = 0;       // Stops save-data pollution on the terminal.
u8 DONT_POLLUTE_READ = 0;

#ifdef TARGET_WEB
#include <emscripten.h>
#endif

extern OSMgrArgs piMgrArgs;

u64 osClockRate = 62500000;

s32 osPiStartDma(UNUSED OSIoMesg *mb, UNUSED s32 priority, UNUSED s32 direction,
                 uintptr_t devAddr, void *vAddr, size_t nbytes,
                 UNUSED OSMesgQueue *mq) {
    memcpy(vAddr, (const void *) devAddr, nbytes);
    return 0;
}

void osSetEventMesg(UNUSED OSEvent e, UNUSED OSMesgQueue *mq, UNUSED OSMesg msg) {
}
s32 osJamMesg(UNUSED OSMesgQueue *mq, UNUSED OSMesg msg, UNUSED s32 flag) {
    return 0;
}
s32 osSendMesg(UNUSED OSMesgQueue *mq, UNUSED OSMesg msg, UNUSED s32 flag) {
    return 0;
}
s32 osRecvMesg(UNUSED OSMesgQueue *mq, UNUSED OSMesg *msg, UNUSED s32 flag) {
    return 0;
}

uintptr_t osVirtualToPhysical(void *addr) {
    return (uintptr_t) addr;
}

void osCreateViManager(UNUSED OSPri pri) {
}
void osViSetMode(UNUSED OSViMode *mode) {
}
void osViSetEvent(UNUSED OSMesgQueue *mq, UNUSED OSMesg msg, UNUSED u32 retraceCount) {
}
void osViBlack(UNUSED u8 active) {
}
void osViSetSpecialFeatures(UNUSED u32 func) {
}
void osViSwapBuffer(UNUSED void *vaddr) {
}

OSTime osGetTime(void) {
    return 0;
}

void osWritebackDCacheAll(void) {
}

void osWritebackDCache(UNUSED void *a, UNUSED size_t b) {
}

void osInvalDCache(UNUSED void *a, UNUSED size_t b) {
}

u32 osGetCount(void) {
    static u32 counter;
    return counter++;
}

s32 osAiSetFrequency(u32 freq) {
    u32 a1;
    s32 a2;
    u32 D_8033491C;

#ifdef VERSION_EU
    D_8033491C = 0x02E6025C;
#else
    D_8033491C = 0x02E6D354;
#endif

    a1 = D_8033491C / (float) freq + .5f;

    if (a1 < 0x84) {
        return -1;
    }

    a2 = (a1 / 66) & 0xff;
    if (a2 > 16) {
        a2 = 16;
    }

    return D_8033491C / (s32) a1;
}

s32 osEepromProbe(UNUSED OSMesgQueue *mq) {
    return 1;
}

s32 osEepromLongRead(UNUSED OSMesgQueue *mq, u8 address, u8 *buffer, int nbytes) {
    u8 content[512];
    s32 ret = -1;

#ifdef TARGET_WEB
    if (EM_ASM_INT({
        var s = localStorage.sm64_save_file;
        if (s && s.length === 684) {
            try {
                var binary = atob(s);
                if (binary.length === 512) {
                    for (var i = 0; i < 512; i++) {
                        HEAPU8[$0 + i] = binary.charCodeAt(i);
                    }
                    return 1;
                }
            } catch (e) {
            }
        }
        return 0;
    }, content)) {
        memcpy(buffer, content + address * 8, nbytes);
        ret = 0;
    }
#else
    DIR* conf_dir = opendir(SDL_GetPrefPath("", "sm64pc"));     // Checks for XDG_DATA_HOME/sm64pc
    char * cur_dir = SDL_GetBasePath();                         // Saves the current working directory so we can return to it later.
    FILE *fp;
    if (ENOENT == errno)  
    {                                                      // XDG_DATA_HOME/sm64pc does not exist, so try to read from the current working directory.
        fp = fopen("sm64_save_file.bin", "rb");
        if (fp == NULL) 
        {
            closedir(conf_dir);  // Save data also not found in cwd, so return an error.
            return -1;
        } 
        else 
        {
            printf("Loading sava data from '%s%s'\n", SDL_GetBasePath(), "sm64_save_file.bin"); // We've found a file in cwd!
        }
    } 
    else 
    {
                    // The config folder exists, so try to read from it.

                chdir(SDL_GetPrefPath("", "sm64pc"));       // Goes to XDG_DATA_HOME/sm64pc.
        fp = fopen("sm64_save_file.bin", "rb");        
        if (fp == NULL)                       // If there's not a file here
        {
            chdir(cur_dir);                
            closedir(conf_dir);                 // We didn't find a save file in XDG_DATA_HOME/sm64pc
            return -1;
        } else printf("Loading save data from '%s%s'\n", SDL_GetPrefPath("", "sm64pc"), "sm64_save_file.bin"); // We've found a file in XDG_DATA_HOME/sm64pc!
            
    }

    
    if (fp == NULL) {
        return -1;
    }
    if (fread(content, 1, 512, fp) == 512) {
        memcpy(buffer, content + address * 8, nbytes);
        ret = 0;
    }
    fclose(fp);
#endif
    return ret;
}

s32 osEepromLongWrite(UNUSED OSMesgQueue *mq, u8 address, u8 *buffer, int nbytes) {
    u8 content[512] = {0};
    if (address != 0 || nbytes != 512) {
        osEepromLongRead(mq, 0, content, 512);
    }
    memcpy(content + address * 8, buffer, nbytes);

#ifdef TARGET_WEB
    EM_ASM({
        var str = "";
        for (var i = 0; i < 512; i++) {
            str += String.fromCharCode(HEAPU8[$0 + i]);
        }
        localStorage.sm64_save_file = btoa(str);
    }, content);
    s32 ret = 0;
#else
    char * cur_dir = SDL_GetBasePath();                         // Current working directory
    DIR* conf_dir = opendir(SDL_GetPrefPath("", "sm64pc"));     // Checks if there's already an 'sm64pc' folder
    if (ENOENT == errno)  // Directory does not exist
    { 
        mkdir(SDL_GetPrefPath("", "sm64pc"), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);    // If there isn't, we'll make one.
        conf_dir = opendir(SDL_GetPrefPath("", "sm64pc"));
        if (errno == ENOENT) 
        {
            printf("Error: Couldn't get save path.\n");                      // If it still doesn't exist, bail out
            exit(ENOENT);
        }
    }

    closedir(conf_dir);
    chdir(SDL_GetPrefPath("", "sm64pc"));                                           // Go to the pref folder
    FILE* fp = fopen("sm64_save_file.bin", "wb");
    if (!DONT_POLLUTE_SAVE) {
        printf("Save data is in '%s%s'\n",SDL_GetPrefPath("", "sm64pc"),  "sm64_save_file.bin");
        DONT_POLLUTE_SAVE = 1;       // This is a bad solution to only print this message once.
    }
    
    chdir(cur_dir);        
    
    if (fp == NULL) {
        return -1;
    }
    s32 ret = fwrite(content, 1, 512, fp) == 512 ? 0 : -1;
    fclose(fp);
#endif
    return ret;
}
