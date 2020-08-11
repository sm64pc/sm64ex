#include <stdio.h>
#include "../network.h"
#include "src/game/level_update.h"
#include "src/game/area.h"
#include "sm64.h"

int matchCount = 0;

extern s16 gMenuMode;

void network_send_level_warp(void) {
    struct Packet p;
    packet_init(&p, PACKET_LEVEL_WARP, true);
    packet_write(&p, &sCurrPlayMode, sizeof(s16));
    packet_write(&p, &sWarpDest, sizeof(struct WarpDest));

    network_send(&p);
}

static void force_well_behaved_state(void) {
    /*
    gDialogBoxState = DIALOG_STATE_OPENING;
    gCourseDoneMenuTimer = 0;
    gCourseCompleteCoins = 0;
    gCourseCompleteCoinsEqual = 0;
    gHudFlash = 0;
    */
    level_set_transition(0, 0);
    gMenuMode = -1;
    gPauseScreenMode = 1;
    gSaveOptSelectIndex = 0;
    gMarioStates[0].action = (gMarioStates[0].pos[1] <= (gMarioStates[0].waterLevel - 100)) ? ACT_WATER_IDLE : ACT_IDLE;
    gCameraMovementFlags &= ~CAM_MOVE_PAUSE_SCREEN;
}

void network_receive_level_warp(struct Packet* p) {
    s16 remotePlayMode;
    struct WarpDest remoteWarpDest;

    packet_read(p, &remotePlayMode, sizeof(s16));
    packet_read(p, &remoteWarpDest, sizeof(struct WarpDest));

    bool matchingDest = memcmp(&remoteWarpDest, &sWarpDest, sizeof(struct WarpDest)) == 0;

    if (remotePlayMode == PLAY_MODE_SYNC_LEVEL && (sCurrPlayMode == PLAY_MODE_NORMAL || sCurrPlayMode == PLAY_MODE_PAUSED)) {
        if (remoteWarpDest.type == WARP_TYPE_NOT_WARPING) { return; }
        sCurrPlayMode = PLAY_MODE_SYNC_LEVEL;
        sWarpDest = remoteWarpDest;
        force_well_behaved_state();
        network_send_level_warp();
        return;
    }

    if (remotePlayMode == PLAY_MODE_SYNC_LEVEL && sCurrPlayMode == PLAY_MODE_SYNC_LEVEL) {
        if (matchingDest) {
            switch (sWarpDest.type) {
                case WARP_TYPE_CHANGE_AREA: sCurrPlayMode = PLAY_MODE_CHANGE_AREA; break;
                case WARP_TYPE_CHANGE_LEVEL: sCurrPlayMode = PLAY_MODE_CHANGE_LEVEL; break;
            }
        } else {
            if (networkType == NT_CLIENT) {
                if (remoteWarpDest.type == WARP_TYPE_NOT_WARPING) { return; }
                // two-player hack: would need to use player index as priority
                sWarpDest = remoteWarpDest;
            }
        }
        network_send_level_warp();
        return;
    }

    if ((remotePlayMode == PLAY_MODE_CHANGE_LEVEL || remotePlayMode == PLAY_MODE_CHANGE_AREA) && sCurrPlayMode == PLAY_MODE_SYNC_LEVEL) {
        if (remoteWarpDest.type == WARP_TYPE_NOT_WARPING) { return; }
        switch (sWarpDest.type) {
            case WARP_TYPE_CHANGE_AREA: sCurrPlayMode = PLAY_MODE_CHANGE_AREA; break;
            case WARP_TYPE_CHANGE_LEVEL: sCurrPlayMode = PLAY_MODE_CHANGE_LEVEL; break;
        }
    }
}
