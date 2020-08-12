#include <stdio.h>
#include "../network.h"
#include "object_fields.h"
#include "object_constants.h"
#include "behavior_table.h"
#include "course_table.h"
#include "src/game/interaction.h"
#include "src/engine/math_util.h"

static u8 localItemId = 1;

// two-player hack: the remoteItemId stuff is only valid for the one remote player
// will need to be extended if MAX_PLAYERS is ever increased
#define MAX_REMOTE_ITEM_IDS 16
static u8 remoteItemIds[MAX_REMOTE_ITEM_IDS] = { 0 };
static u8 onRemoteItemId = 0;

static f32 dist_to_pos(struct Object* o, f32* pos) {
    f32 x = (f32)o->oPosX - pos[0]; x *= x;
    f32 y = (f32)o->oPosY - pos[1]; y *= y;
    f32 z = (f32)o->oPosZ - pos[2]; z *= z;
    return (f32)sqrt(x + y + z);
}

static struct Object* find_nearest_item(const BehaviorScript *behavior, f32* pos, float minDist) {
    uintptr_t *behaviorAddr = segmented_to_virtual(behavior);
    struct Object *closestObj = NULL;
    struct Object *obj;
    struct ObjectNode *listHead;

    extern struct ObjectNode *gObjectLists;
    listHead = &gObjectLists[get_object_list_from_behavior(behaviorAddr)];
    obj = (struct Object *) listHead->next;

    while (obj != (struct Object *) listHead) {
        if (obj->behavior == behaviorAddr && obj->activeFlags != ACTIVE_FLAG_DEACTIVATED && !(obj->oInteractStatus & INT_STATUS_INTERACTED)) {
            f32 objDist = dist_to_pos(obj, pos);
            if (objDist < minDist) {
                closestObj = obj;
                minDist = objDist;
            }
        }
        obj = (struct Object *) obj->header.next;
    }

    return closestObj;
}

void network_send_collect_item(struct Object* o) {
    enum BehaviorId behaviorId = get_id_from_behavior(o->behavior);

    struct Packet p;
    packet_init(&p, PACKET_COLLECT_ITEM, true);
    packet_write(&p, &localItemId, sizeof(u8));
    packet_write(&p, &behaviorId, sizeof(enum BehaviorId));
    packet_write(&p, &o->oPosX, sizeof(f32) * 3);

    network_send(&p);
    localItemId++;
}

void network_receive_collect_item(struct Packet* p) {
    u8 remoteItemId = 0;
    enum BehaviorId behaviorId;
    void* behavior = NULL;
    f32 pos[3] = { 0 };

    packet_read(p, &remoteItemId, sizeof(u8));
    packet_read(p, &behaviorId, sizeof(enum BehaviorId));
    packet_read(p, &pos, sizeof(f32) * 3);

    behavior = get_behavior_from_id(behaviorId);

    // check if remote item id has already been seen
    for (int i = 0; i < MAX_REMOTE_ITEM_IDS; i++) {
        if (remoteItemIds[i] == remoteItemId) {
            // we already saw this item!
            return;
        }
    }
    // cache the seen id
    remoteItemIds[onRemoteItemId] = remoteItemId;
    onRemoteItemId = (onRemoteItemId + 1) % MAX_REMOTE_ITEM_IDS;

    // make sure it's valid
    if (behavior == NULL) { return; }

    // find the item
    struct Object* item = find_nearest_item(behavior, pos, 1000);
    if (item == NULL) { return; }

    // destroy item
    item->oInteractStatus = INT_STATUS_INTERACTED;
}
