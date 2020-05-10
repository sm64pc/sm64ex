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

#endif
