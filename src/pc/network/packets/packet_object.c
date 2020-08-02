#include <stdio.h>
#include "../network.h"
#include "object_fields.h"
#include "object_constants.h"

u32 nextSyncID = 1;
struct Object* syncObject = NULL;

float player_distance(struct MarioState* marioState, struct Object* obj) {
    if (marioState->marioObj == NULL) { return 0; }
    f32 mx = marioState->marioObj->header.gfx.pos[0] - obj->oPosX;
    f32 my = marioState->marioObj->header.gfx.pos[1] - obj->oPosY;
    f32 mz = marioState->marioObj->header.gfx.pos[2] - obj->oPosZ;
    mx *= mx;
    my *= my;
    mz *= mz;
    return sqrt(mx + my + mz);
}

void network_init_object(struct Object *object) {
    object->oSyncID = nextSyncID++;
    syncObject = object;
}

void network_send_object(struct Object* o) {
    o = syncObject;
    if (o == NULL) { return; }
    if (o->activeFlags == ACTIVE_FLAG_DEACTIVATED) { return; }
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
    if (syncObject == NULL) { return; }
    u32 syncId;
    packet_read(p, &syncId, 4);
    packet_read(p, &syncObject->oPosX, 28);
    packet_read(p, &syncObject->oAction, 4);
    packet_read(p, &syncObject->oHeldState, 4);
    packet_read(p, &syncObject->oMoveAngleYaw, 4);
}
