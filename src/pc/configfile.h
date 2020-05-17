#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include <stdbool.h>

#define MAX_BINDS    3
#define MAX_VOLUME   127
#define VOLUME_SHIFT 7

extern bool         configFullscreen;
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
extern bool 		cheatMoonjump;
extern bool 		cheatGodmode;
extern bool 		cheatEnablecheats;
extern bool			cheatInfinitelives;
extern bool			cheatSuperspeed;
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

void configfile_load(const char *filename);
void configfile_save(const char *filename);

#endif
