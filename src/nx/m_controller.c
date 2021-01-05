#ifdef TARGET_SWITCH
#include "m_controller.h"

#include <switch.h>

PadState pad;
u32 target_device = -1;
HidVibrationValue VibrationValue;
HidVibrationValue VibrationValue_stop;
HidVibrationValue VibrationValues[2];
HidVibrationDeviceHandle VibrationDeviceHandles[2][2];

u32 last = 0;
u32 vlength = 0;

void controller_nx_init(){
    padInitializeDefault(&pad);    
    
    hidInitializeVibrationDevices(VibrationDeviceHandles[0], 2, HidNpadIdType_Handheld, HidNpadStyleTag_NpadHandheld);
    hidInitializeVibrationDevices(VibrationDeviceHandles[1], 2, HidNpadIdType_No1, HidNpadStyleTag_NpadJoyDual);
}

void get_controller_nx(struct NXController* controller){
    
    u32 tagStyle = padGetStyleSet(&pad);

    if (tagStyle & HidNpadStyleTag_NpadFullKey) {
        controller->name = "Pro Controller";
        controller->type = PRO_CONTROLLER;
        controller->icon = "textures/icons/pro_controller";
    }
    else if (tagStyle & HidNpadStyleTag_NpadHandheld) {
        controller->name = "Nintendo Switch";
        controller->type = HANDHELD;
        controller->icon = "textures/icons/joycons";
    } else {
        controller->name = "Dual Joy-Con";
        controller->type = DUAL_JOYCON;
        controller->icon = "textures/icons/joycons";
    }
}

void controller_nx_rumble_play(f32 strength, f32 length) {
    memset(VibrationValues, 0, sizeof(VibrationValues));

    VibrationValue.freq_low  = 160.0f;
    VibrationValue.freq_high = 320.0f;
    VibrationValue.amp_low   = strength;
    VibrationValue.amp_high  = strength;

    memcpy(&VibrationValues[0], &VibrationValue, sizeof(HidVibrationValue));
    memcpy(&VibrationValues[1], &VibrationValue, sizeof(HidVibrationValue));
    last = 0;
    vlength = (length / 1000);
}

void controller_nx_rumble_stop(void) {
    memset(&VibrationValue_stop, 0, sizeof(HidVibrationValue));

    VibrationValue_stop.freq_low  = 160.0f;
    VibrationValue_stop.freq_high = 320.0f;

    memcpy(&VibrationValues[0], &VibrationValue_stop, sizeof(HidVibrationValue));
    memcpy(&VibrationValues[1], &VibrationValue_stop, sizeof(HidVibrationValue));

    hidSendVibrationValues(VibrationDeviceHandles[target_device], VibrationValues, 2);        
    hidSendVibrationValues(VibrationDeviceHandles[1-target_device], VibrationValues, 2);
    vlength = -1;
}



void controller_nx_rumble_loop(void){
    padUpdate(&pad);
    target_device = padIsHandheld(&pad) ? 0 : 1;
    if(vlength != -1 && last <= vlength){
        hidSendVibrationValues(VibrationDeviceHandles[target_device], VibrationValues, 2);        
        last++;
    } else vlength = -1;
}

#endif
