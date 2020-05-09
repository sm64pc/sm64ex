#ifndef CONFIGFILE_H
#define CONFIGFILE_H
#include <stdbool.h>
#include "engine/math_util.h"

extern bool         configFullscreen;
extern unsigned int configKeyA;
extern unsigned int configKeyB;
extern unsigned int configKeyStart;
extern unsigned int configKeyR;
extern unsigned int configKeyZ;
extern unsigned int configKeyCUp;
extern unsigned int configKeyCDown;
extern unsigned int configKeyCLeft;
extern unsigned int configKeyCRight;
extern unsigned int configKeyStickUp;
extern unsigned int configKeyStickDown;
extern unsigned int configKeyStickLeft;
extern unsigned int configKeyStickRight;

extern u8 configCameraXSens;
extern u8 configCameraYSens;
extern u8 configCameraAggr;
extern u8 configCameraPan;
extern u8 configCameraInvertX;
extern u8 configCameraInvertY;
extern u8 configEnableCamera;
extern u8 configCameraMouse;

void configfile_load(const char *filename);
void configfile_save(const char *filename);

#endif
