#include <stdio.h>
#include "../network.h"
#include "src/game/level_update.h"
#include "src/game/area.h"

extern struct WarpNode gPaintingWarpNode;
extern u8 sSelectableStarIndex;
extern u8 sSelectedActIndex;

struct PacketDataInsidePainting {
    u8 insidePainting;
    u8 controlPainting;
    u8 starIndex;
    u8 actIndex;
    struct WarpNode warpNode;
};

static clock_t lastSentTime = 0;
static float minUpdateRate = 0.5f;
static struct PacketDataInsidePainting lastSentData = { 0 };

static void populate_packet_data(struct PacketDataInsidePainting* data) {
    data->insidePainting = gInsidePainting;
    data->controlPainting = gControlPainting;
    data->starIndex = sSelectableStarIndex;
    data->actIndex = sSelectedActIndex;
    data->warpNode = gPaintingWarpNode;
}

void network_send_inside_painting(void) {
    struct PacketDataInsidePainting data = { 0 };
    populate_packet_data(&data);

    struct Packet p;
    packet_init(&p, PACKET_INSIDE_PAINTING, false);
    packet_write(&p, &data, sizeof(struct PacketDataInsidePainting));
    network_send(&p);

    lastSentData = data;
    lastSentTime = clock();
}

void network_receive_inside_painting(struct Packet* p) {
    struct PacketDataInsidePainting remote = { 0 };
    packet_read(p, &remote, sizeof(struct PacketDataInsidePainting));

    if (networkType == NT_CLIENT && gControlPainting && remote.controlPainting) {
        // we both think we should control the painting, host wins the tie
        gControlPainting = false;
    }

    if (!gControlPainting && remote.controlPainting) {
        // update star/act index to show the one in control's selection
        sSelectableStarIndex = remote.starIndex;
        sSelectedActIndex = remote.actIndex;
    }

    // see if the warp nodes are the same
    int compareNodes = memcmp(&gPaintingWarpNode, &remote.warpNode, sizeof(struct WarpNode));

    if (gControlPainting && !remote.controlPainting && (compareNodes == 0)) {
        // remote is well behaved now, we can control the painting
        gWaitingForRemotePainting = false;
    }

    bool shouldJumpInside = !gControlPainting && (!gInsidePainting && remote.insidePainting);

    // ERROR: THE DESTINATION MISMATCH DOESN'T MOVE THE CLIENT TO THE CORRECT SCREEN!
    bool destinationMismatch = !gControlPainting && (compareNodes != 0);

    if (shouldJumpInside || destinationMismatch) {
        initiate_painting_warp_node(&remote.warpNode, true);
        set_play_mode(PLAY_MODE_CHANGE_LEVEL);
    }

    if (gControlPainting && !remote.controlPainting && !gInsidePainting && remote.insidePainting) {
        // we're in control and no longer in the painting, let remote know
        network_send_inside_painting();
    }

    if (!gControlPainting && remote.controlPainting && !remote.insidePainting) {
        // remote is in control and in game, we should be too
        star_select_finish_selection();
    }
}

void network_update_inside_painting(void) {
    struct PacketDataInsidePainting data = { 0 };
    populate_packet_data(&data);
    int compareData = memcmp(&data, &lastSentData, sizeof(struct PacketDataInsidePainting));

    float timeSinceSend = (clock() - lastSentTime) / CLOCKS_PER_SEC;

    if (compareData != 0 || timeSinceSend > minUpdateRate) {
        network_send_inside_painting();
    }
}