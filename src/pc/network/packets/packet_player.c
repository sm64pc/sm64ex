#include <stdio.h>
#include "../network.h"

void network_send_player(void) {
    if (gMarioStates[0].marioObj == NULL) { return; }
    struct Packet p;
    packet_init(&p, PACKET_PLAYER);
    packet_write(&p, &gMarioStates[0], 96);
    packet_write(&p, gMarioStates[0].controller, 20);
    packet_write(&p, gMarioStates[0].marioObj->rawData.asU32, 320);
    network_send(&p);
}

void network_receive_player(struct Packet* p) {
    if (gMarioStates[1].marioObj == NULL) { return; }
    int oldActionState = gMarioStates[1].actionState;

    packet_read(p, &gMarioStates[1], 96);
    packet_read(p, gMarioStates[1].controller, 20);
    packet_read(p, &gMarioStates[1].marioObj->rawData.asU32, 320);

    // restore action state, needed for jump kicking
    gMarioStates[1].actionState = oldActionState;
}
