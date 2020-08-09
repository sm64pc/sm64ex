#include <stdio.h>
#include <limits.h>
#include "../network.h"
#include "object_fields.h"
#include "object_constants.h"
#include "behavior_data.h"
#include "behavior_table.h"

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
    so->clockSinceUpdate = clock();
    so->extraFieldCount = 0;
    so->behavior = o->behavior;
    so->onEventId = 0;
    so->fullObjectSync = false;
    so->keepRandomSeed = false;
    so->maxUpdateRate = 0;
    so->ignore_if_true = NULL;
    memset(so->extraFields, 0, sizeof(void*) * MAX_SYNC_OBJECT_FIELDS);
}

void network_object_settings(struct Object *o, bool fullObjectSync, float maxUpdateRate, bool keepRandomSeed, u8 ignore_if_true(struct Object*)) {
    assert(o->oSyncID != 0);
    struct SyncObject* so = &syncObjects[o->oSyncID];
    so->fullObjectSync = fullObjectSync;
    so->maxUpdateRate = maxUpdateRate;
    so->keepRandomSeed = keepRandomSeed;
    so->ignore_if_true = ignore_if_true;
}

void network_init_object_field(struct Object *o, void* field) {
    assert(o->oSyncID != 0);
    struct SyncObject* so = &syncObjects[o->oSyncID];
    int index = so->extraFieldCount++;
    so->extraFields[index] = field;
}

bool network_owns_object(struct Object* o) {
    struct SyncObject* so = &syncObjects[o->oSyncID];
    if (so == NULL) { return false; }
    return so->owned;
}

void network_send_object(struct Object* o) {
    struct SyncObject* so = &syncObjects[o->oSyncID];
    if (so == NULL) { return; }

    so->onEventId++;

    enum BehaviorId behaviorId = get_id_from_behavior(o->behavior);
    bool reliable = (o->activeFlags == ACTIVE_FLAG_DEACTIVATED || so->maxSyncDistance == SYNC_DISTANCE_ONLY_EVENTS);

    struct Packet p;
    packet_init(&p, PACKET_OBJECT, reliable);
    packet_write(&p, &o->oSyncID, 4);
    packet_write(&p, &so->onEventId, sizeof(u16));
    packet_write(&p, &behaviorId, sizeof(enum BehaviorId));

    if (so->maxSyncDistance != SYNC_DISTANCE_ONLY_EVENTS) {
        packet_write(&p, &o->activeFlags, sizeof(s16));
        packet_write(&p, &o->header.gfx.node.flags, sizeof(s16));
    }

    if (so->fullObjectSync) {
        packet_write(&p, o->rawData.asU32, sizeof(u32) * 80);
    } else {

        if (so->maxSyncDistance != SYNC_DISTANCE_ONLY_EVENTS) {
            packet_write(&p, &o->oPosX, sizeof(u32) * 7);
            packet_write(&p, &o->oAction, sizeof(u32));
            packet_write(&p, &o->oSubAction, sizeof(u32));
            packet_write(&p, &o->oInteractStatus, sizeof(u32));
            packet_write(&p, &o->oHeldState, sizeof(u32));
            packet_write(&p, &o->oMoveAngleYaw, sizeof(u32));
            packet_write(&p, &o->oTimer, sizeof(u32));
        }

        packet_write(&p, &so->extraFieldCount, sizeof(u8));
        for (int i = 0; i < so->extraFieldCount; i++) {
            assert(so->extraFields[i] != NULL);
            packet_write(&p, so->extraFields[i], sizeof(u32));
        }
    }

    so->clockSinceUpdate = clock();

    if (o->activeFlags == ACTIVE_FLAG_DEACTIVATED) { forget_sync_object(so); }

    if (o->behavior != so->behavior) {
        printf("network_send_object() BEHAVIOR MISMATCH!\n");
        forget_sync_object(so);
        return;
    }

    network_send(&p);
}

void network_receive_object(struct Packet* p) {
    // get sync ID
    u32 syncId;
    packet_read(p, &syncId, sizeof(u32));
    assert(syncId < MAX_SYNC_OBJECTS);

    // retrieve SyncObject
    struct SyncObject* so = &syncObjects[syncId];
    so->clockSinceUpdate = clock();
    if (so->ignore_if_true != NULL && (*so->ignore_if_true)(so->o)) { return; }

    // extract Object
    struct Object* o = syncObjects[syncId].o;
    if (o == NULL) { printf("%s failed to receive object!\n", NETWORKTYPESTR); return; }

    // make sure it's active
    if (o->activeFlags == ACTIVE_FLAG_DEACTIVATED) {
        return;
    }

    // make sure this is the newest event possible
    volatile u16 eventId = 0;
    packet_read(p, &eventId, sizeof(u16));
    if (so->onEventId > eventId && (u16)abs(eventId - so->onEventId) < USHRT_MAX / 2) { return; }
    so->onEventId = eventId;

    // make sure the behaviors match
    enum BehaviorId behaviorId;
    packet_read(p, &behaviorId, sizeof(enum BehaviorId));
    so->behavior = get_behavior_from_id(behaviorId);
    if (o->behavior != so->behavior) {
        printf("network_receive_object() BEHAVIOR MISMATCH!\n");
        forget_sync_object(so);
        return;
    }

    // sync only death
    if (so->maxSyncDistance == SYNC_DISTANCE_ONLY_DEATH) {
        s16 activeFlags;
        packet_read(p, &activeFlags, sizeof(u16));
        if (activeFlags == ACTIVE_FLAG_DEACTIVATED) {
            so->o->oSyncDeath = 1;
            forget_sync_object(so);
        }
        return;
    }

    if (gMarioStates[0].heldObj == o) {
        return;
    }

    // write object flags
    if (so->maxSyncDistance != SYNC_DISTANCE_ONLY_EVENTS) {
        packet_read(p, &o->activeFlags, sizeof(u16));
        packet_read(p, &o->header.gfx.node.flags, sizeof(s16));
    }

    if (so->fullObjectSync) {
        packet_read(p, o->rawData.asU32, sizeof(u32) * 80);
    } else {

        if (so->maxSyncDistance != SYNC_DISTANCE_ONLY_EVENTS) {
            packet_read(p, &o->oPosX, sizeof(u32) * 7);
            packet_read(p, &o->oAction, sizeof(u32));
            packet_read(p, &o->oSubAction, sizeof(u32));
            packet_read(p, &o->oInteractStatus, sizeof(u32));
            packet_read(p, &o->oHeldState, sizeof(u32));
            packet_read(p, &o->oMoveAngleYaw, sizeof(u32));
            packet_read(p, &o->oTimer, sizeof(u32));
        }

        // write extra fields
        u8 extraFields = 0;
        packet_read(p, &extraFields, sizeof(u8));
        assert(extraFields == so->extraFieldCount);
        for (int i = 0; i < extraFields; i++) {
            assert(so->extraFields[i] != NULL);
            packet_read(p, so->extraFields[i], sizeof(u32));
        }

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

        // check if we should be the one syncing this object
        so->owned = should_own_object(so);
        if (!so->owned) { continue; }

        // check update rate
        if (so->maxSyncDistance == SYNC_DISTANCE_ONLY_DEATH) {
            if (so->o->activeFlags != ACTIVE_FLAG_DEACTIVATED) { continue; }
            network_send_object(syncObjects[i].o);
            continue;
        }

        float dist = player_distance(&gMarioStates[0], so->o);
        if (so->maxSyncDistance != SYNC_DISTANCE_INFINITE && dist > so->maxSyncDistance) { continue; }
        float updateRate = dist / 1000.0f;
        if (gMarioStates[0].heldObj == so->o) { updateRate = 0; }

        if (so->maxUpdateRate > 0 && updateRate < so->maxUpdateRate) { updateRate = so->maxUpdateRate; }
        if (updateRate < 0.33f) { updateRate = 0.33f; }

        float timeSinceUpdate = ((float)clock() - (float)so->clockSinceUpdate) / (float)CLOCKS_PER_SEC;
        if (timeSinceUpdate < updateRate) { continue; }

        network_send_object(syncObjects[i].o);
    }

}
