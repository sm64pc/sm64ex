#include <stdio.h>
#include "../network.h"
#include "behavior_table.h"
#include "course_table.h"
#include "object_fields.h"
#include "object_constants.h"
#include "game/interaction.h"

extern s16 gCurrSaveFileNum;
extern s16 gCurrCourseNum;

static f32 dist_to_pos(struct Object* o, f32* pos) {
    f32 x = (f32)o->oPosX - pos[0]; x *= x;
    f32 y = (f32)o->oPosY - pos[1]; y *= y;
    f32 z = (f32)o->oPosZ - pos[2]; z *= z;
    return (f32)sqrt(x + y + z);
}

static struct Object* find_nearest_star(const BehaviorScript* behavior, f32* pos, float minDist) {
    uintptr_t* behaviorAddr = segmented_to_virtual(behavior);
    struct Object* closestObj = NULL;
    struct Object* obj;
    struct ObjectNode* listHead;

    extern struct ObjectNode* gObjectLists;
    listHead = &gObjectLists[get_object_list_from_behavior(behaviorAddr)];
    obj = (struct Object*) listHead->next;

    while (obj != (struct Object*) listHead) {
        if (obj->behavior == behaviorAddr && obj->activeFlags != ACTIVE_FLAG_DEACTIVATED) {
            f32 objDist = dist_to_pos(obj, pos);
            if (objDist < minDist) {
                closestObj = obj;
                minDist = objDist;
            }
        }
        obj = (struct Object*) obj->header.next;
    }

    return closestObj;
}

void network_send_collect_star(struct Object* o, s16 coinScore, s16 starIndex) {
    enum BehaviorId behaviorId = get_id_from_behavior(o->behavior);

    struct Packet p;
    packet_init(&p, PACKET_COLLECT_STAR, true);

    packet_write(&p, &gCurrSaveFileNum, sizeof(s16));
    packet_write(&p, &gCurrCourseNum, sizeof(s16));
    packet_write(&p, &o->oPosX, sizeof(u32) * 3);
    packet_write(&p, &behaviorId, sizeof(enum BehaviorId));
    packet_write(&p, &coinScore, sizeof(s16));
    packet_write(&p, &starIndex, sizeof(s16));

    network_send(&p);
}

void network_receive_collect_star(struct Packet* p) {
    u32 pos[3] = { 0 };
    enum BehaviorId behaviorId;
    void* behavior = NULL;
    s16 coinScore, starIndex;
    s16 lastSaveFileNum = gCurrSaveFileNum;
    s16 lastCourseNum = gCurrCourseNum;

    packet_read(p, &gCurrSaveFileNum, sizeof(s16));
    packet_read(p, &gCurrCourseNum, sizeof(s16));
    packet_read(p, &pos, sizeof(u32) * 3);
    packet_read(p, &behaviorId, sizeof(enum BehaviorId));
    packet_read(p, &coinScore, sizeof(s16));
    packet_read(p, &starIndex, sizeof(s16));

    behavior = get_behavior_from_id(behaviorId);

    save_file_collect_star_or_key(coinScore, starIndex);

    s32 numStars = save_file_get_total_star_count(gCurrSaveFileNum - 1, COURSE_MIN - 1, COURSE_MAX - 1);
    for (int i = 0; i < MAX_PLAYERS; i++) {
        gMarioStates[i].numStars = numStars;
    }

    gCurrSaveFileNum = lastSaveFileNum;
    gCurrCourseNum = lastCourseNum;

    struct Object* star = find_nearest_star(behavior, pos, 500);
    if (star != NULL) {
        star->oInteractStatus = INT_STATUS_INTERACTED;
    }
}
