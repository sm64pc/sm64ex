#include "lib/src/libultra_internal.h"
#include "lib/src/osContInternal.h"
#include "macros.h"

#include "../configfile.h"

#include "controller_recorded_tas.h"
#include "controller_keyboard.h"
#include "controller_sdl.h"

// Analog camera movement by Pathétique (github.com/vrmiguel), y0shin and Mors
// Contribute or communicate bugs at github.com/vrmiguel/sm64-analog-camera

static struct ControllerAPI *controller_implementations[] = {
    &controller_recorded_tas,
    #if defined(CAPI_SDL2) || defined(CAPI_SDL1)
    &controller_sdl,
    #endif
    &controller_keyboard,
};

s32 osContInit(UNUSED OSMesgQueue *mq, u8 *controllerBits, UNUSED OSContStatus *status) {
    for (size_t i = 0; i < sizeof(controller_implementations) / sizeof(struct ControllerAPI *); i++)
        controller_implementations[i]->init();
    *controllerBits = 1;
    return 0;
}

s32 osMotorStart(UNUSED void *pfs) {
    // Since rumble stops by osMotorStop, its duration is not nessecary.
    // Set it to 5 seconds and hope osMotorStop() is called in time.
    if (configRumbleStrength)
        controller_rumble_play(configRumbleStrength / 100.0f, 5.0f);
    return 0;
}

s32 osMotorStop(UNUSED void *pfs) {
    if (configRumbleStrength)
        controller_rumble_stop();
    return 0;
}

u32 osMotorInit(UNUSED OSMesgQueue *mq, UNUSED void *pfs, UNUSED s32 port) {
    return 0; // rumble is initialized in the specific backend's init function
}

s32 osContStartReadData(UNUSED OSMesgQueue *mesg) {
    return 0;
}

void osContGetReadData(OSContPad *pad) {
    pad->button = 0;
    pad->stick_x = 0;
    pad->stick_y = 0;
    pad->ext_stick_x = 0;
    pad->ext_stick_y = 0;
    pad->errnum = 0;

    for (size_t i = 0; i < sizeof(controller_implementations) / sizeof(struct ControllerAPI *); i++) {
        controller_implementations[i]->read(pad);
    }
}

u32 controller_get_raw_key(void) {
    for (size_t i = 0; i < sizeof(controller_implementations) / sizeof(struct ControllerAPI *); i++) {
        u32 vk = controller_implementations[i]->rawkey();
        if (vk != VK_INVALID) return vk + controller_implementations[i]->vkbase;
    }
    return VK_INVALID;
}

void controller_shutdown(void) {
    for (size_t i = 0; i < sizeof(controller_implementations) / sizeof(struct ControllerAPI *); i++) {
        if (controller_implementations[i]->shutdown)
            controller_implementations[i]->shutdown();
    }
}

void controller_reconfigure(void) {
    for (size_t i = 0; i < sizeof(controller_implementations) / sizeof(struct ControllerAPI *); i++) {
        if (controller_implementations[i]->reconfig)
            controller_implementations[i]->reconfig();
    }
}

void controller_rumble_play(float str, float time) {
    for (size_t i = 0; i < sizeof(controller_implementations) / sizeof(struct ControllerAPI *); i++) {
        if (controller_implementations[i]->rumble_play)
            controller_implementations[i]->rumble_play(str, time);
    }
}

void controller_rumble_stop(void) {
    for (size_t i = 0; i < sizeof(controller_implementations) / sizeof(struct ControllerAPI *); i++) {
        if (controller_implementations[i]->rumble_stop)
            controller_implementations[i]->rumble_stop();
    }
}
