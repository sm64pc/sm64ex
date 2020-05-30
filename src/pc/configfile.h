#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include <stdbool.h>

#define CONFIGFILE_DEFAULT "sm64config.txt"

#define MAX_BINDS    3
#define MAX_VOLUME   127
#define VOLUME_SHIFT 7

typedef struct {
    unsigned int x, y, w, h;
    unsigned int vsync;
    bool reset;
    bool fullscreen;
    bool exiting_fullscreen;
    bool settings_changed;
} ConfigWindow;

extern ConfigWindow configWindow;
extern unsigned int configFiltering;
extern unsigned int configMasterVolume;
extern unsigned int configKeyA[];
extern unsigned int configKeyB[];
extern unsigned int configKeyStart[];
extern unsigned int configKeyL[];
extern unsigned int configKeyR[];
extern unsigned int configKeyZ[];
extern unsigned int configKeyCUp[];
extern unsigned int configKeyCDown[];
extern unsigned int configKeyCLeft[];
extern unsigned int configKeyCRight[];
extern unsigned int configKeyStickUp[];
extern unsigned int configKeyStickDown[];
extern unsigned int configKeyStickLeft[];
extern unsigned int configKeyStickRight[];
extern unsigned int configStickDeadzone;
#ifdef EXTERNAL_TEXTURES
extern bool         configPrecacheRes;
#endif
#ifdef BETTERCAMERA
extern unsigned int configCameraXSens;
extern unsigned int configCameraYSens;
extern unsigned int configCameraAggr;
extern unsigned int configCameraPan;
extern unsigned int configCameraDegrade;
extern bool         configCameraInvertX;
extern bool         configCameraInvertY;
extern bool         configEnableCamera;
extern bool         configCameraMouse;
#endif
extern bool         configHUD;

void configfile_load(const char *filename);
void configfile_save(const char *filename);
const char *configfile_name(void);

#endif
