#include <stdio.h>
#include "../network.h"
#include "src/game/level_update.h"
#include "src/game/area.h"

int warpTimeout = 0;

void network_send_level_warp(void) {
    struct Packet p;
    packet_init(&p, PACKET_LEVEL_WARP, false);
    packet_write(&p, &sCurrPlayMode, 2);
    packet_write(&p, &gCurrLevelNum, 2);
    packet_write(&p, &sDelayedWarpArg, 4);
    packet_write(&p, &sSourceWarpNodeId, 2);

    network_send(&p);
}

void network_receive_level_warp(struct Packet* p) {
    if (warpTimeout != 0) { return; }
    s16 remotePlayMode;
    s16 remoteLevelNum;
    s32 remoteWarpArg;
    s16 remoteWarpNodeId;

    packet_read(p, &remotePlayMode, 2);
    packet_read(p, &remoteLevelNum, 2);
    packet_read(p, &remoteWarpArg, 4);
    packet_read(p, &remoteWarpNodeId, 2);

    if (gCurrLevelNum == remoteLevelNum) {
        if (sCurrPlayMode == PLAY_MODE_SYNC_LEVEL) {
            // our levels match now, lets play!
            set_play_mode(PLAY_MODE_NORMAL);
            set_menu_mode((s16)-1);
        }
        // our levels match, make sure the other player knows that
        network_send_level_warp();
        return;
    }

    // remote isn't trying to sync, don't warp
    if (remotePlayMode != PLAY_MODE_SYNC_LEVEL) { return; }

    // we're trying to sync, don't warp
    if (sCurrPlayMode == PLAY_MODE_SYNC_LEVEL) { return; }

    // warp to the level
    sDelayedWarpTimer = 1;
    sDelayedWarpArg = remoteWarpArg;
    sSourceWarpNodeId = remoteWarpNodeId;
    sDelayedWarpOp = WARP_OP_FORCE_SYNC;

    // don't repeat the warp too quickly
    warpTimeout = 2;
}

void network_update_level_warp(void) {
    network_send_level_warp();
    if (warpTimeout > 0) { warpTimeout--; }
}