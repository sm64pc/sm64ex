#include <stdio.h>
#include "../network.h"
#include "src/game/level_update.h"
#include "src/game/area.h"

int matchCount = 0;

void network_send_level_warp(void) {
    struct Packet p;
    packet_init(&p, PACKET_LEVEL_WARP, false);
    packet_write(&p, &sCurrPlayMode, sizeof(s16));
    packet_write(&p, &gCurrLevelNum, sizeof(s16));
    packet_write(&p, &sDelayedWarpArg, sizeof(s32));
    packet_write(&p, &sSourceWarpNodeId, sizeof(s16));

    network_send(&p);
}

void network_receive_level_warp(struct Packet* p) {
    s16 remotePlayMode;
    s16 remoteLevelNum;
    s32 remoteWarpArg;
    s16 remoteWarpNodeId;

    packet_read(p, &remotePlayMode, sizeof(s16));
    packet_read(p, &remoteLevelNum, sizeof(s16));
    packet_read(p, &remoteWarpArg, sizeof(s32));
    packet_read(p, &remoteWarpNodeId, sizeof(s16));

    bool matching = (remoteLevelNum == gCurrLevelNum)
                 && (remoteWarpArg == sDelayedWarpArg)
                 && (remoteWarpNodeId == sSourceWarpNodeId);

    if (matching) {
        if (sCurrPlayMode == PLAY_MODE_SYNC_LEVEL) {
            // our levels match now, lets play!
            set_play_mode(PLAY_MODE_NORMAL);
            set_menu_mode((s16)-1);
        }
        // our levels match, make sure the other player knows that
        if (matchCount++ < 3) {
            network_send_level_warp();
        } else {
            matchCount = 0;
        }
        return;
    }
    matchCount = 0;

    // remote isn't trying to sync, don't warp
    if (remotePlayMode != PLAY_MODE_SYNC_LEVEL) { return; }

    // we're trying to sync, don't warp
    if (sCurrPlayMode == PLAY_MODE_SYNC_LEVEL) { return; }

    // warp to the level
    sDelayedWarpTimer = 1;
    sDelayedWarpArg = remoteWarpArg;
    sSourceWarpNodeId = remoteWarpNodeId;
    sDelayedWarpOp = WARP_OP_FORCE_SYNC;
}

void network_update_level_warp(void) {
    network_send_level_warp();
}
