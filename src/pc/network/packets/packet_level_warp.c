#include <stdio.h>
#include "../network.h"
#include "src/game/level_update.h"
#include "src/game/area.h"

int matchCount = 0;

extern s16 gMenuMode;

void network_send_level_warp(void) {
    struct Packet p;
    packet_init(&p, PACKET_LEVEL_WARP, true);
    packet_write(&p, &sCurrPlayMode, sizeof(s16));
    packet_write(&p, &sWarpDest, sizeof(struct WarpDest));

    network_send(&p);
}

void network_receive_level_warp(struct Packet* p) {
    s16 remotePlayMode;
    struct WarpDest remoteWarpDest;

    packet_read(p, &remotePlayMode, sizeof(s16));
    packet_read(p, &remoteWarpDest, sizeof(struct WarpDest));

    bool matchingDest = memcmp(&remoteWarpDest, &sWarpDest, sizeof(struct WarpDest)) == 0;

    if (remotePlayMode == PLAY_MODE_SYNC_LEVEL && (sCurrPlayMode == PLAY_MODE_NORMAL || sCurrPlayMode == PLAY_MODE_PAUSED)) {
        sCurrPlayMode = PLAY_MODE_SYNC_LEVEL;
        sWarpDest = remoteWarpDest;
        gMenuMode = -1;
        gPauseScreenMode = 1;
        if (sTransitionTimer < 1) { sTransitionTimer = 1; }
        gCameraMovementFlags &= ~CAM_MOVE_PAUSE_SCREEN;
        network_send_level_warp();
        return;
    }

    if (remotePlayMode == PLAY_MODE_SYNC_LEVEL && sCurrPlayMode == PLAY_MODE_SYNC_LEVEL) {
        if (matchingDest) {
            sCurrPlayMode = PLAY_MODE_CHANGE_LEVEL;
        } else {
            if (networkType == NT_CLIENT) {
                // two-player hack: would need to use player index as priority
                sWarpDest = remoteWarpDest;
            }
        }
        network_send_level_warp();
        return;
    }

    if (remotePlayMode == PLAY_MODE_CHANGE_LEVEL && sCurrPlayMode == PLAY_MODE_SYNC_LEVEL) {
        sCurrPlayMode = PLAY_MODE_CHANGE_LEVEL;
    }
}
