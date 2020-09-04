#include <stdio.h>
#include "../network.h"
#include "object_fields.h"
#include "object_constants.h"
#include "src/game/object_helpers.h"
#include "behavior_data.h"
#include "behavior_table.h"

static u8 localSpawnId = 1;

// two-player hack: the remoteSpawnId stuff is only valid for the one remote player
// will need to be extended if MAX_PLAYERS is ever increased
#define MAX_REMOTE_SPAWN_IDS 16
static u8 remoteSpawnIds[MAX_REMOTE_SPAWN_IDS] = { 0 };
static u8 onRemoteSpawnId = 0;

#define MAX_SPAWN_OBJECTS_PER_PACKET 8

struct SpawnObjectData {
    u8 parentId;
    u32 model;
    enum BehaviorId behaviorId;
    s16 activeFlags;
    s32 rawData[80];
};

static u8 generate_parent_id(struct Object* objects[], u8 onIndex) {
    struct Object* o = objects[onIndex];

    // special case if the parent is itself
    if (o->parentObj == o) { return (u8)-1; }

    if (onIndex == 0) {
        assert(o->parentObj->oSyncID != 0);
        return (u8)o->parentObj->oSyncID;
    }

    for (u8 i = onIndex; i != (u8)-1; i--) {
        if (o->parentObj == objects[i]) { return i; }
    }

    assert(false);
}

void network_send_spawn_objects(struct Object* objects[], u32 models[], u8 objectCount) {
    assert(objectCount < MAX_SPAWN_OBJECTS_PER_PACKET);

    struct Packet p;
    packet_init(&p, PACKET_SPAWN_OBJECTS, true);
    packet_write(&p, &localSpawnId, sizeof(u8));
    packet_write(&p, &objectCount, sizeof(u8));

    for (u8 i = 0; i < objectCount; i++) {
        struct Object* o = objects[i];
        u32 model = models[i];
        u8 parentId = generate_parent_id(objects, i);
        enum BehaviorId behaviorId = get_id_from_behavior(o->behavior);
        packet_write(&p, &parentId, sizeof(u8));
        packet_write(&p, &model, sizeof(u32));
        packet_write(&p, &behaviorId, sizeof(enum BehaviorId));
        packet_write(&p, &o->activeFlags, sizeof(s16));
        packet_write(&p, o->rawData.asU32, sizeof(s32) * 80);
    }

    network_send(&p);

    localSpawnId++;
    if (localSpawnId == 0) { localSpawnId++; }
}

void network_receive_spawn_objects(struct Packet* p) {
    u8 remoteSpawnId = 0;
    u8 objectCount = 0;

    packet_read(p, &remoteSpawnId, sizeof(u8));
    packet_read(p, &objectCount, sizeof(u8));

    // check if remote spawn id has already been seen
    for (u16 i = 0; i < MAX_REMOTE_SPAWN_IDS; i++) {
        if (remoteSpawnIds[i] == remoteSpawnId) {
            // we already saw this event!
            return;
        }
    }
    // cache the seen id
    remoteSpawnIds[onRemoteSpawnId] = remoteSpawnId;
    onRemoteSpawnId = (onRemoteSpawnId + 1) % MAX_REMOTE_SPAWN_IDS;

    // two-player hack
    u8 reserveId = (networkLevelLoaded && networkType == NT_SERVER) ? 1 : 0;
    bool receivedReservedSyncObject = false;

    struct Object* spawned[MAX_SPAWN_OBJECTS_PER_PACKET] = { 0 };
    for (u8 i = 0; i < objectCount; i++) {
        struct SpawnObjectData data = { 0 };
        packet_read(p, &data.parentId, sizeof(u8));
        packet_read(p, &data.model, sizeof(u32));
        packet_read(p, &data.behaviorId, sizeof(enum BehaviorId));
        packet_read(p, &data.activeFlags, sizeof(s16));
        packet_read(p, &data.rawData, sizeof(s32) * 80);

        struct Object* parentObj = NULL;
        if (data.parentId == (u8)-1) {
            // this object is it's own parent, set it to a known object temporarily
            parentObj = gMarioStates[0].marioObj;
        } else {
            // this object has a known parent
            parentObj = (i == 0)
                      ? syncObjects[data.parentId].o
                      : spawned[data.parentId];
            if (parentObj == NULL) { continue; }
        }

        void* behavior = (void*)get_behavior_from_id(data.behaviorId);
        struct Object* o = spawn_object(parentObj, data.model, behavior);
        memcpy(o->rawData.asU32, data.rawData, sizeof(u32) * 80);

        // correct the temporary parent with the object itself
        if (data.parentId == (u8)-1) { o->parentObj = o; }

        // they've allocated one of their reserved sync objects
        if (o->oSyncID != 0 && syncObjects[o->oSyncID].reserved == reserveId) {
            syncObjects[o->oSyncID].o = o;
            syncObjects[o->oSyncID].reserved = 0;
            receivedReservedSyncObject = true;
        }

        spawned[i] = o;
    }

    // update their block of reserved ids
    if (networkType == NT_SERVER && receivedReservedSyncObject) {
        network_send_reservation();
    }
}
