#include "lib/src/libultra_internal.h"
#include "lib/src/osContInternal.h"

#include "controller_recorded_tas.h"
#include "controller_keyboard.h"

#include "controller_sdl.h"

// Analog camera movement by Pathétique (github.com/vrmiguel), y0shin and Mors
// Contribute or communicate bugs at github.com/vrmiguel/sm64-analog-camera

int16_t rightx;
int16_t righty;
int c_rightx;
int c_righty;

static struct ControllerAPI *controller_implementations[] = {
    &controller_recorded_tas,
    &controller_sdl,
    &controller_keyboard,
};

s32 osContInit(OSMesgQueue *mq, u8 *controllerBits, OSContStatus *status) {
    for (size_t i = 0; i < sizeof(controller_implementations) / sizeof(struct ControllerAPI *); i++) {
        controller_implementations[i]->init();
    }
    *controllerBits = 1;
    return 0;
}

s32 osContStartReadData(OSMesgQueue *mesg) {
    return 0;
}

void osContGetReadData(OSContPad *pad) {
    pad->button = 0;
    pad->stick_x = 0;
    pad->stick_y = 0;
    pad->errnum = 0;

#ifdef BETTERCAMERA
    uint32_t magnitude_sq = (uint32_t)(rightx * rightx) + (uint32_t)(righty * righty);
    if (magnitude_sq > (uint32_t)(DEADZONE * DEADZONE)) {
        c_rightx = rightx / 0x100;
        int stick_y = -righty / 0x100;
        c_righty = stick_y == 128 ? 127 : stick_y;
    }
#endif


    for (size_t i = 0; i < sizeof(controller_implementations) / sizeof(struct ControllerAPI *); i++) {
        controller_implementations[i]->read(pad);
    }
}
