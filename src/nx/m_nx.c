#ifdef TARGET_SWITCH

#include <switch.h>

#include "m_nx.h"

int socketID = -1;

void initNX(){
    appletInitializeGamePlayRecording();
    socketInitializeDefault();
    socketID = nxlinkStdio();
    appletSetGamePlayRecordingState(1);

    Result rc = psmInitialize();
    if (R_FAILED(rc)) psmExit();
}

void exitNX(){
    socketExit();
    appletSetGamePlayRecordingState(0);
}

float getBatteryPercentage(){
    u32 charge;
    psmGetBatteryChargePercentage(&charge);
    return (charge / 100.0f);
}

#endif
