#include <stdio.h>
#include <limits.h>
#include "../network.h"
#include "object_fields.h"
#include "object_constants.h"
#include "behavior_data.h"
#include "behavior_table.h"

static u8 nextSyncID = 1;
struct SyncObject syncObjects[MAX_SYNC_OBJECTS] = { 0 };

// todo: move this to somewhere more general
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

void network_clear_sync_objects(void) {
    for (int i = 0; i < MAX_SYNC_OBJECTS; i++) {
        network_forget_sync_object(&syncObjects[i]);
    }
    nextSyncID = 1;
}

void network_set_sync_id(struct Object* o) {
    if (o->oSyncID != 0) { return; }

    // two-player hack
    u8 reserveId = (networkLevelLoaded && networkType == NT_CLIENT) ? 1 : 0;

    for (int i = 0; i < MAX_SYNC_OBJECTS; i++) {
        if (syncObjects[nextSyncID].reserved == reserveId && syncObjects[nextSyncID].o == NULL) { break; }
        nextSyncID = (nextSyncID + 1) % MAX_SYNC_OBJECTS;
    }
    assert(syncObjects[nextSyncID].o == NULL);
    assert(syncObjects[nextSyncID].reserved == reserveId);
    o->oSyncID = nextSyncID;
    nextSyncID = (nextSyncID + 1) % MAX_SYNC_OBJECTS;

    assert(o->oSyncID < MAX_SYNC_OBJECTS);
}

struct SyncObject* network_init_object(struct Object *o, float maxSyncDistance) {
    // generate new sync ID
    network_set_sync_id(o);

    // set default values for sync object
    struct SyncObject* so = &syncObjects[o->oSyncID];
    so->o = o;
    so->reserved = 0;
    so->maxSyncDistance = maxSyncDistance;
    so->owned = false;
    so->clockSinceUpdate = clock();
    so->extraFieldCount = 0;
    so->behavior = o->behavior;
    so->rxEventId = 0;
    so->txEventId = 0;
    so->fullObjectSync = false;
    so->keepRandomSeed = false;
    so->maxUpdateRate = 0;
    so->ignore_if_true = NULL;
    so->syncDeathEvent = true;
    memset(so->extraFields, 0, sizeof(void*) * MAX_SYNC_OBJECT_FIELDS);

    return so;
}

void network_init_object_field(struct Object *o, void* field) {
    assert(o->oSyncID != 0);
    // remember to synchronize this extra field
    struct SyncObject* so = &syncObjects[o->oSyncID];
    int index = so->extraFieldCount++;
    so->extraFields[index] = field;
}

bool network_owns_object(struct Object* o) {
    struct SyncObject* so = &syncObjects[o->oSyncID];
    if (so == NULL) { return false; }
    return so->owned;
}

bool network_sync_object_initialized(struct Object* o) {
    if (o->oSyncID == 0) { return false; }
    if (syncObjects[o->oSyncID].behavior == NULL) { return false; }
    return true;
}

// ----- header ----- //

static void packet_write_object_header(struct Packet* p, struct Object* o) {
    struct SyncObject* so = &syncObjects[o->oSyncID];
    enum BehaviorId behaviorId = get_id_from_behavior(o->behavior);

    packet_write(p, &o->oSyncID, sizeof(u32));
    packet_write(p, &so->txEventId, sizeof(u16));
    packet_write(p, &behaviorId, sizeof(enum BehaviorId));
}

static struct SyncObject* packet_read_object_header(struct Packet* p) {
    // get sync ID, sanity check
    u32 syncId = 0;
    packet_read(p, &syncId, sizeof(u32));
    if (syncId == 0 || syncId >= MAX_SYNC_OBJECTS) {
        printf("%s invalid SyncID!\n", NETWORKTYPESTR, syncId);
        return NULL;
    }

    // extract object, sanity check
    struct Object* o = syncObjects[syncId].o;
    if (o == NULL) {
        printf("%s invalid SyncObject!\n", NETWORKTYPESTR);
        return NULL;
    }

    // retrieve SyncObject, check if we should update using callback
    struct SyncObject* so = &syncObjects[syncId];
    if (so->ignore_if_true != NULL && (*so->ignore_if_true)(so->o)) {
        return NULL;
    }
    so->clockSinceUpdate = clock();

    // make sure this is the newest event possible
    u16 eventId = 0;
    packet_read(p, &eventId, sizeof(u16));
    if (so->rxEventId > eventId && (u16)abs(eventId - so->rxEventId) < USHRT_MAX / 2) {
        return NULL;
    }
    so->rxEventId = eventId;

    // make sure the behaviors match
    enum BehaviorId behaviorId;
    packet_read(p, &behaviorId, sizeof(enum BehaviorId));
    so->behavior = get_behavior_from_id(behaviorId);
    if (o->behavior != so->behavior) {
        printf("network_receive_object() behavior mismatch!\n");
        network_forget_sync_object(so);
        return NULL;
    }

    return so;
}

// ----- full sync ----- //

static void packet_write_object_full_sync(struct Packet* p, struct Object* o) {
    struct SyncObject* so = &syncObjects[o->oSyncID];
    if (!so->fullObjectSync) { return; }

    // write all of raw data
    packet_write(p, o->rawData.asU32, sizeof(u32) * 80);
}

static void packet_read_object_full_sync(struct Packet* p, struct Object* o) {
    struct SyncObject* so = &syncObjects[o->oSyncID];
    if (!so->fullObjectSync) { return; }

    // read all of raw data
    packet_read(p, o->rawData.asU32, sizeof(u32) * 80);
}

// ----- standard fields ----- //

static void packet_write_object_standard_fields(struct Packet* p, struct Object* o) {
    struct SyncObject* so = &syncObjects[o->oSyncID];
    if (so->fullObjectSync) { return; }
    if (so->maxSyncDistance == SYNC_DISTANCE_ONLY_DEATH) { return; }
    if (so->maxSyncDistance == SYNC_DISTANCE_ONLY_EVENTS) { return; }

    // write the standard fields
    packet_write(p, &o->oPosX, sizeof(u32) * 7);
    packet_write(p, &o->oAction, sizeof(u32));
    packet_write(p, &o->oSubAction, sizeof(u32));
    packet_write(p, &o->oInteractStatus, sizeof(u32));
    packet_write(p, &o->oHeldState, sizeof(u32));
    packet_write(p, &o->oMoveAngleYaw, sizeof(u32));
    packet_write(p, &o->oTimer, sizeof(u32));
    packet_write(p, &o->activeFlags, sizeof(s16));
    packet_write(p, &o->header.gfx.node.flags, sizeof(s16));
}

static void packet_read_object_standard_fields(struct Packet* p, struct Object* o) {
    struct SyncObject* so = &syncObjects[o->oSyncID];
    if (so->fullObjectSync) { return; }
    if (so->maxSyncDistance == SYNC_DISTANCE_ONLY_DEATH) { return; }
    if (so->maxSyncDistance == SYNC_DISTANCE_ONLY_EVENTS) { return; }

    // read the standard fields
    packet_read(p, &o->oPosX, sizeof(u32) * 7);
    packet_read(p, &o->oAction, sizeof(u32));
    packet_read(p, &o->oSubAction, sizeof(u32));
    packet_read(p, &o->oInteractStatus, sizeof(u32));
    packet_read(p, &o->oHeldState, sizeof(u32));
    packet_read(p, &o->oMoveAngleYaw, sizeof(u32));
    packet_read(p, &o->oTimer, sizeof(u32));
    packet_read(p, &o->activeFlags, sizeof(u16));
    packet_read(p, &o->header.gfx.node.flags, sizeof(s16));
}

// ----- extra fields ----- //

static void packet_write_object_extra_fields(struct Packet* p, struct Object* o) {
    struct SyncObject* so = &syncObjects[o->oSyncID];
    if (so->fullObjectSync) { return; }
    if (so->maxSyncDistance == SYNC_DISTANCE_ONLY_DEATH) { return; }

    // write the count
    packet_write(p, &so->extraFieldCount, sizeof(u8));

    // write the extra field
    for (int i = 0; i < so->extraFieldCount; i++) {
        assert(so->extraFields[i] != NULL);
        packet_write(p, so->extraFields[i], sizeof(u32));
    }
}

static void packet_read_object_extra_fields(struct Packet* p, struct Object* o) {
    struct SyncObject* so = &syncObjects[o->oSyncID];
    if (so->fullObjectSync) { return; }
    if (so->maxSyncDistance == SYNC_DISTANCE_ONLY_DEATH) { return; }

    // read the count and sanity check
    u8 extraFieldsCount = 0;
    packet_read(p, &extraFieldsCount, sizeof(u8));
    if (extraFieldsCount != so->extraFieldCount) {
        return;
    }

    // read the extra fields
    for (int i = 0; i < extraFieldsCount; i++) {
        assert(so->extraFields[i] != NULL);
        packet_read(p, so->extraFields[i], sizeof(u32));
    }
}

// ----- only death ----- //

static void packet_write_object_only_death(struct Packet* p, struct Object* o) {
    struct SyncObject* so = &syncObjects[o->oSyncID];
    if (so->maxSyncDistance != SYNC_DISTANCE_ONLY_DEATH) { return; }
    packet_write(p, &o->activeFlags, sizeof(s16));
}

static void packet_read_object_only_death(struct Packet* p, struct Object* o) {
    struct SyncObject* so = &syncObjects[o->oSyncID];
    if (so->maxSyncDistance != SYNC_DISTANCE_ONLY_DEATH) { return; }
    s16 activeFlags;
    packet_read(p, &activeFlags, sizeof(u16));
    if (activeFlags == ACTIVE_FLAG_DEACTIVATED) {
        // flag the object as dead, the behavior is responsible for clean up
        so->o->oSyncDeath = 1;
        network_forget_sync_object(so);
    }
}

// ----- main send/receive ----- //

void network_send_object(struct Object* o) {
    // sanity check SyncObject
    if (!network_sync_object_initialized(o)) { return; }
    struct SyncObject* so = &syncObjects[o->oSyncID];
    if (so == NULL) { return; }
    if (o->behavior != so->behavior) {
        printf("network_send_object() BEHAVIOR MISMATCH!\n");
        network_forget_sync_object(so);
        return;
    }
    if (o != so->o) {
        printf("network_send_object() OBJECT MISMATCH!\n");
        network_forget_sync_object(so);
        return;
    }

    bool reliable = (o->activeFlags == ACTIVE_FLAG_DEACTIVATED || so->maxSyncDistance == SYNC_DISTANCE_ONLY_EVENTS);
    network_send_object_reliability(o, reliable);
}

void network_send_object_reliability(struct Object* o, bool reliable) {
    // sanity check SyncObject
    if (!network_sync_object_initialized(o)) { return; }
    struct SyncObject* so = &syncObjects[o->oSyncID];
    if (so == NULL) { return; }
    if (o->behavior != so->behavior) {
        printf("network_send_object() BEHAVIOR MISMATCH!\n");
        network_forget_sync_object(so);
        return;
    }
    if (o != so->o) {
        printf("network_send_object() OBJECT MISMATCH!\n");
        network_forget_sync_object(so);
        return;
    }

    // always send a new event ID
    so->txEventId++;
    so->clockSinceUpdate = clock();

    // write the packet data
    struct Packet p;
    packet_init(&p, PACKET_OBJECT, reliable);
    packet_write_object_header(&p, o);
    packet_write_object_full_sync(&p, o);
    packet_write_object_standard_fields(&p, o);
    packet_write_object_extra_fields(&p, o);
    packet_write_object_only_death(&p, o);

    // check for object death
    if (o->activeFlags == ACTIVE_FLAG_DEACTIVATED) {
        network_forget_sync_object(so);
    }

    // send the packet out
    network_send(&p);
}

void network_receive_object(struct Packet* p) {
    // read the header and sanity check the packet
    struct SyncObject* so = packet_read_object_header(p);
    if (so == NULL) { return; }
    struct Object* o = so->o;
    if (!network_sync_object_initialized(o)) { return; }

    // make sure no one can update an object we're holding
    if (gMarioStates[0].heldObj == o) { return; }

    // read the rest of the packet data
    packet_read_object_full_sync(p, o);
    packet_read_object_standard_fields(p, o);
    packet_read_object_extra_fields(p, o);
    packet_read_object_only_death(p, o);

    // deactivated
    if (o->activeFlags == ACTIVE_FLAG_DEACTIVATED) {
        network_forget_sync_object(so);
    }
}

bool should_own_object(struct SyncObject* so) {
    if (so->o->oHeldState == HELD_HELD && so->o->heldByPlayerIndex == 0) { return true; }
    if (player_distance(&gMarioStates[0], so->o) > player_distance(&gMarioStates[1], so->o)) { return false; }
    if (so->o->oHeldState == HELD_HELD && so->o->heldByPlayerIndex != 0) { return false; }
    return true;
}

void network_forget_sync_object(struct SyncObject* so) {
    so->o = NULL;
    so->behavior = NULL;
    so->reserved = 0;
    so->owned = false;
}

void network_update_objects(void) {
    for (int i = 1; i < nextSyncID; i++) {
        struct SyncObject* so = &syncObjects[i];
        if (so->o == NULL) { continue; }

        // check for stale sync object
        if (so->o->oSyncID != i) {
            printf("ERROR! Sync ID mismatch!\n");
            network_forget_sync_object(so);
            continue;
        }

        // check if we should be the one syncing this object
        so->owned = should_own_object(so);
        if (!so->owned) { continue; }

        // check for 'only death' event
        if (so->maxSyncDistance == SYNC_DISTANCE_ONLY_DEATH) {
            if (so->o->activeFlags != ACTIVE_FLAG_DEACTIVATED) { continue; }
            network_send_object(syncObjects[i].o);
            continue;
        }

        // calculate the update rate
        float dist = player_distance(&gMarioStates[0], so->o);
        if (so->maxSyncDistance != SYNC_DISTANCE_INFINITE && dist > so->maxSyncDistance) { continue; }
        float updateRate = dist / 1000.0f;
        if (gMarioStates[0].heldObj == so->o) { updateRate = 0; }

        // set max and min update rate
        if (so->maxUpdateRate > 0 && updateRate < so->maxUpdateRate) { updateRate = so->maxUpdateRate; }
        if (updateRate < 0.33f) { updateRate = 0.33f; }

        // see if we should update
        float timeSinceUpdate = ((float)clock() - (float)so->clockSinceUpdate) / (float)CLOCKS_PER_SEC;
        if (timeSinceUpdate < updateRate) { continue; }

        // update!
        network_send_object(syncObjects[i].o);
    }

}
