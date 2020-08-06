#include <stdio.h>
#include "../network.h"
#include "object_fields.h"
#include "object_constants.h"

u8 nextSyncID = 1;
struct SyncObject syncObjects[MAX_SYNC_OBJECTS] = { 0 };

void network_init_object(struct Object *o, float maxSyncDistance) {
    if (o->oSyncID == 0) {
        for (int i = 0; i < MAX_SYNC_OBJECTS; i++) {
            if (syncObjects[nextSyncID].o == NULL) { break; }
            nextSyncID = (nextSyncID + 1) % MAX_SYNC_OBJECTS;
        }
        assert(syncObjects[nextSyncID].o == NULL);
        o->oSyncID = nextSyncID;
        nextSyncID = (nextSyncID + 1) % MAX_SYNC_OBJECTS;
    }
    assert(o->oSyncID < MAX_SYNC_OBJECTS);
    struct SyncObject* so = &syncObjects[o->oSyncID];
    so->o = o;
    so->maxSyncDistance = maxSyncDistance;
    so->owned = false;
    so->ticksSinceUpdate = -1;
    so->extraFieldCount = 0;
    so->behavior = o->behavior;
    memset(so->extraFields, 0, sizeof(void*) * MAX_SYNC_OBJECT_FIELDS);
}

void network_init_object_field(struct Object *o, void* field) {
    assert(o->oSyncID != 0);
    struct SyncObject* so = &syncObjects[o->oSyncID];
    int index = so->extraFieldCount++;
    so->extraFields[index] = field;
}

void network_send_object(struct Object* o) {
    struct SyncObject* so = &syncObjects[o->oSyncID];
    if (so == NULL) { return; }

    bool reliable = (o->activeFlags == ACTIVE_FLAG_DEACTIVATED);
    struct Packet p;
    packet_init(&p, PACKET_OBJECT, reliable);
    packet_write(&p, &o->oSyncID, 4);
    packet_write(&p, &so->behavior, sizeof(void*));
    packet_write(&p, &o->activeFlags, 2);
    packet_write(&p, &o->oPosX, 28);
    packet_write(&p, &o->oAction, 4);
    packet_write(&p, &o->oHeldState, 4);
    packet_write(&p, &o->oMoveAngleYaw, 4);

    packet_write(&p, &so->extraFieldCount, 1);
    for (int i = 0; i < so->extraFieldCount; i++) {
        assert(so->extraFields[i] != NULL);
        packet_write(&p, so->extraFields[i], 4);
    }

    so->ticksSinceUpdate = 0;

    if (o->activeFlags == ACTIVE_FLAG_DEACTIVATED) { forget_sync_object(so); }

    if (o->behavior != so->behavior) {
        printf("network_send_object() BEHAVIOR MISMATCH!\n");
    }

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

    // make sure it's active
    if (o->activeFlags == ACTIVE_FLAG_DEACTIVATED) {
        return;
    }

    // sync only death
    if (so->maxSyncDistance == SYNC_DISTANCE_ONLY_DEATH) {
        s16 activeFlags;
        packet_read(p, &activeFlags, 2);
        if (activeFlags == ACTIVE_FLAG_DEACTIVATED) {
            so->o->oSyncDeath = 1;
            forget_sync_object(so);
        }
        return;
    }

    // write object flags
    packet_read(p, &so->behavior, sizeof(void*));
    packet_read(p, &o->activeFlags, 2);
    packet_read(p, &o->oPosX, 28);
    packet_read(p, &o->oAction, 4);
    packet_read(p, &o->oHeldState, 4);
    packet_read(p, &o->oMoveAngleYaw, 4);

    // write extra fields
    u8 extraFields = 0;
    packet_read(p, &extraFields, 1);
    assert(extraFields == so->extraFieldCount);
    for (int i = 0; i < extraFields; i++) {
        assert(so->extraFields[i] != NULL);
        packet_read(p, so->extraFields[i], 4);
    }

    if (o->behavior != so->behavior) {
        printf("network_receive_object() BEHAVIOR MISMATCH!\n");
    }

    // deactivated
    if (o->activeFlags == ACTIVE_FLAG_DEACTIVATED) {
        forget_sync_object(so);
    }
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
    if (so->o->oHeldState == HELD_HELD && so->o->heldByPlayerIndex == 0) { return true; }
    if (player_distance(&gMarioStates[0], so->o) > player_distance(&gMarioStates[1], so->o)) { return false; }
    if (so->o->oHeldState == HELD_HELD && so->o->heldByPlayerIndex != 0) { return false; }
    return true;
}

void forget_sync_object(struct SyncObject* so) {
    so->o = NULL;
    so->owned = false;
    so->ticksSinceUpdate = -1;
}

void network_update_objects(void) {
    for (int i = 0; i < MAX_SYNC_OBJECTS; i++) {
        struct SyncObject* so = &syncObjects[i];
        if (so->o == NULL) { continue; }

        // check for stale sync object
        if (so->o->oSyncID != i) {
            printf("ERROR! Sync ID mismatch!\n");
            forget_sync_object(so);
            continue;
        }
        so->ticksSinceUpdate++;

        // check if we should be the one syncing this object
        if (!should_own_object(so)) { continue; }

        // check update rate
        if (so->maxSyncDistance == SYNC_DISTANCE_ONLY_DEATH) {
            if (so->o->activeFlags != ACTIVE_FLAG_DEACTIVATED) { continue; }
            network_send_object(syncObjects[i].o);
            continue;
        }

        float dist = player_distance(&gMarioStates[0], so->o);
        if (so->maxSyncDistance != SYNC_DISTANCE_INFINITE && dist > so->maxSyncDistance) { continue; }
        int updateRate = dist / 50;
        if (gMarioStates[0].heldObj == so->o) { updateRate = 0; }
        if (so->ticksSinceUpdate < updateRate) { continue; }

        network_send_object(syncObjects[i].o);
    }

}