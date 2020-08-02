#include <stdio.h>
#include "../network.h"
#include "object_fields.h"
#include "object_constants.h"

u32 nextSyncID = 1;
struct Object* syncObjects[MAX_SYNC_OBJECTS] = { 0 };

float player_distance(struct MarioState* marioState, struct Object* o) {
    if (marioState->marioObj == NULL) { return 0; }
    f32 mx = marioState->marioObj->header.gfx.pos[0] - o->oPosX;
    f32 my = marioState->marioObj->header.gfx.pos[1] - o->oPosY;
    f32 mz = marioState->marioObj->header.gfx.pos[2] - o->oPosZ;
    mx *= mx;
    my *= my;
    mz *= mz;
    return sqrt(mx + my + mz);
}

void network_init_object(struct Object *o) {
    if (o->oSyncID == 0) {
        o->oSyncID = nextSyncID++;
    }
    assert(o->oSyncID < MAX_SYNC_OBJECTS);
    syncObjects[o->oSyncID] = o;
}

void network_send_object(struct Object* o) {
    int expectedID = 1;
    o = syncObjects[expectedID];
    if (o == NULL) { return; }
    //if (o->activeFlags == ACTIVE_FLAG_DEACTIVATED) { return; }
    if (o->oSyncID != expectedID) { return; }
    if (player_distance(&gMarioStates[0], o) > player_distance(&gMarioStates[1], o)) { return; }

    struct Packet p;
    packet_init(&p, PACKET_OBJECT);
    packet_write(&p, &o->oSyncID, 4);
    packet_write(&p, &o->oPosX, 28);
    packet_write(&p, &o->oAction, 4);
    packet_write(&p, &o->oHeldState, 4);
    packet_write(&p, &o->oMoveAngleYaw, 4);

    network_send(&p);
}

void network_receive_object(struct Packet* p) {
    u32 syncId;
    packet_read(p, &syncId, 4);
    assert(syncId < MAX_SYNC_OBJECTS);
    struct Object* o = syncObjects[syncId];
    assert(o != NULL);

    packet_read(p, &o->oPosX, 28);
    packet_read(p, &o->oAction, 4);
    packet_read(p, &o->oHeldState, 4);
    packet_read(p, &o->oMoveAngleYaw, 4);
}
