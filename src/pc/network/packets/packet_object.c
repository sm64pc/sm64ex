#include <stdio.h>
#include "../network.h"
#include "object_fields.h"
#include "object_constants.h"

u32 nextSyncID = 1;
struct SyncObject syncObjects[MAX_SYNC_OBJECTS] = { 0 };

void network_init_object(struct Object *o) {
    if (o->oSyncID == 0) {
        o->oSyncID = nextSyncID++;
    }
    assert(o->oSyncID < MAX_SYNC_OBJECTS);
    syncObjects[o->oSyncID].o = o;
    syncObjects[o->oSyncID].owned = false;
    syncObjects[o->oSyncID].ticksSinceUpdate = -1;
    syncObjects[o->oSyncID].syncDeactive = 0;
}

void network_send_object(struct SyncObject* so) {
    struct Object* o = so->o;

    struct Packet p;
    packet_init(&p, PACKET_OBJECT);
    packet_write(&p, &o->oSyncID, 4);
    packet_write(&p, &o->activeFlags, 2);
    packet_write(&p, &o->oPosX, 28);
    packet_write(&p, &o->oAction, 4);
    packet_write(&p, &o->oHeldState, 4);
    packet_write(&p, &o->oMoveAngleYaw, 4);

    if (o->activeFlags == ACTIVE_FLAG_DEACTIVATED) { so->syncDeactive++; }
    so->ticksSinceUpdate = 0;
    network_send(&p);
}

void network_receive_object(struct Packet* p) {
    // get sync ID
    u32 syncId;
    packet_read(p, &syncId, 4);
    assert(syncId < MAX_SYNC_OBJECTS);

    // retrieve SyncObject
    struct SyncObject* so = &syncObjects[syncId];
    so->ticksSinceUpdate = 0;

    // extract Object
    struct Object* o = syncObjects[syncId].o;
    if (o == NULL) { printf("%s failed to receive object!\n", NETWORKTYPESTR); return; }

    // write object flags
    packet_read(p, &o->activeFlags, 2);
    packet_read(p, &o->oPosX, 28);
    packet_read(p, &o->oAction, 4);
    packet_read(p, &o->oHeldState, 4);
    packet_read(p, &o->oMoveAngleYaw, 4);

    if (o->activeFlags == ACTIVE_FLAG_DEACTIVATED) { so->syncDeactive++; }
}

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

bool should_own_object(struct SyncObject* so) {
    if (player_distance(&gMarioStates[0], so->o) > player_distance(&gMarioStates[1], so->o)) { return false; }
    if (so->o->oHeldState == HELD_HELD && so->o->heldByPlayerIndex != 0) { return false; }
    return true;
}

void forget_sync_object(struct SyncObject* so) {
    so->o = NULL;
    so->owned = false;
    so->ticksSinceUpdate = -1;
    so->syncDeactive = 0;
}

void network_update_objects(void) {
    for (int i = 0; i < MAX_SYNC_OBJECTS; i++) {
        struct SyncObject* so = &syncObjects[i];
        if (so->o == NULL) { continue; }

        // check for stale sync object
        if (so->o->oSyncID != i || so->syncDeactive > 10) {
            forget_sync_object(so);
            continue;
        }
        so->ticksSinceUpdate++;

        // check if we should be the one syncing this object
        if (!should_own_object(so)) { continue; }

        // check update rate
        int updateRate = player_distance(&gMarioStates[0], so->o) / 50;
        if (gMarioStates[0].heldObj == so->o) { updateRate = 0; }
        if (so->ticksSinceUpdate < updateRate) { continue; }

        network_send_object(&syncObjects[i]);
    }

}