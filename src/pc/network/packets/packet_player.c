#include <stdio.h>
#include "../network.h"
#include "object_fields.h"
#include "object_constants.h"

void network_send_player(void) {
    if (gMarioStates[0].marioObj == NULL) { return; }
    u32 heldSyncID = (gMarioStates[0].heldObj != NULL)
                   ? gMarioStates[0].heldObj->oSyncID
                   : NULL;

    struct Packet p;
    packet_init(&p, PACKET_PLAYER, false);
    packet_write(&p, &gMarioStates[0], 96);
    packet_write(&p, gMarioStates[0].controller, 20);
    packet_write(&p, gMarioStates[0].marioObj->rawData.asU32, 320);
    packet_write(&p, &gMarioStates[0].health, 2);

    packet_write(&p, &heldSyncID, 4);
    network_send(&p);
}

void network_receive_player(struct Packet* p) {
    if (gMarioStates[1].marioObj == NULL) { return; }
    int oldActionState = gMarioStates[1].actionState;
    u32 heldSyncID = NULL;

    packet_read(p, &gMarioStates[1], 96);
    packet_read(p, gMarioStates[1].controller, 20);
    packet_read(p, &gMarioStates[1].marioObj->rawData.asU32, 320);
    packet_write(p, &gMarioStates[1].health, 2);
    packet_read(p, &heldSyncID, 4);

    if (heldSyncID != NULL) {
        assert(syncObjects[heldSyncID].o != NULL);
        gMarioStates[1].heldObj = syncObjects[heldSyncID].o;
        gMarioStates[1].heldObj->heldByPlayerIndex = 1;
    } else {
        gMarioStates[1].heldObj = NULL;
    }

    // restore action state, needed for jump kicking
    gMarioStates[1].actionState = oldActionState;
}

void network_update_player(void) {
    network_send_player();
}
