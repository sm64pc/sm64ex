#ifndef CONTROLLER_API
#define CONTROLLER_API

#define DEADZONE 4960

// Analog camera movement by Pathétique (github.com/vrmiguel), y0shin and Mors
// Contribute or communicate bugs at github.com/vrmiguel/sm64-analog-camera

#include <ultra64.h>

struct ControllerAPI {
    void (*init)(void);
    void (*read)(OSContPad *pad);
};

    // Declaring these variables here save them for later use. They were originally declared in controller_sdl.c.
int16_t rightx;
int16_t righty;

int c_rightx;   // Will be converted into [-128 ~ 128] range. 
int c_righty;   

#endif
