#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include <stdbool.h>

#define CONFIG_FILE "sm64config.txt"
#define MAX_BINDS 3

extern bool         configFullscreen;
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
#ifdef BETTERCAMERA
extern unsigned int configCameraXSens;
extern unsigned int configCameraYSens;
extern unsigned int configCameraAggr;
extern unsigned int configCameraPan;
extern bool         configCameraInvertX;
extern bool         configCameraInvertY;
extern bool         configEnableCamera;
extern bool         configCameraMouse;
#endif
#ifdef DISCORDRPC
extern bool         configDiscordRPC;
#endif

void configfile_load(const char *filename);
void configfile_save(const char *filename);

#endif
