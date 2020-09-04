#include <stdio.h>
#include "../network.h"
#include "object_fields.h"
#include "object_constants.h"
#include "behavior_table.h"
#include "course_table.h"
#include "src/game/interaction.h"
#include "src/engine/math_util.h"
#include "src/game/memory.h"
#include "src/game/object_helpers.h"

// defined in sparkle_spawn_star.inc.c
void bhv_spawn_star_no_level_exit(struct Object* object, u32 sp20);

static u8 localCoinId = 1;

// two-player hack: the remoteCoinId stuff is only valid for the one remote player
// will need to be extended if MAX_PLAYERS is ever increased
#define MAX_REMOTE_COIN_IDS 16
static u8 remoteCoinIds[MAX_REMOTE_COIN_IDS] = { 0 };
static u8 onRemoteCoinId = 0;

static f32 dist_to_pos(struct Object* o, f32* pos) {
    f32 x = (f32)o->oPosX - pos[0]; x *= x;
    f32 y = (f32)o->oPosY - pos[1]; y *= y;
    f32 z = (f32)o->oPosZ - pos[2]; z *= z;
    return (f32)sqrt(x + y + z);
}

static struct Object* find_nearest_coin(const BehaviorScript *behavior, f32* pos, s32 coinValue, float minDist) {
    uintptr_t *behaviorAddr = segmented_to_virtual(behavior);
    struct Object *closestObj = NULL;
    struct Object *obj;
    struct ObjectNode *listHead;

    extern struct ObjectNode *gObjectLists;
    listHead = &gObjectLists[get_object_list_from_behavior(behaviorAddr)];
    obj = (struct Object *) listHead->next;

    while (obj != (struct Object *) listHead) {
        if (obj->behavior == behaviorAddr && obj->activeFlags != ACTIVE_FLAG_DEACTIVATED && obj->oDamageOrCoinValue == coinValue) {
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

void network_send_collect_coin(struct Object* o) {
    enum BehaviorId behaviorId = get_id_from_behavior(o->behavior);

    struct Packet p;
    packet_init(&p, PACKET_COLLECT_COIN, true);
    packet_write(&p, &localCoinId, sizeof(u8));
    packet_write(&p, &behaviorId, sizeof(enum BehaviorId));
    packet_write(&p, &o->oPosX, sizeof(f32) * 3);
    packet_write(&p, &gMarioStates[0].numCoins, sizeof(s16));
    packet_write(&p, &o->oDamageOrCoinValue, sizeof(s32));

    network_send(&p);
    localCoinId++;
}

void network_receive_collect_coin(struct Packet* p) {
    u8 remoteCoinId = 0;
    enum BehaviorId behaviorId;
    f32 pos[3] = { 0 };
    s16 numCoins = 0;
    s32 coinValue = 0;

    packet_read(p, &remoteCoinId, sizeof(u8));
    packet_read(p, &behaviorId, sizeof(enum BehaviorId));
    packet_read(p, &pos, sizeof(f32) * 3);
    packet_read(p, &numCoins, sizeof(s16));
    packet_read(p, &coinValue, sizeof(s32));

    const void* behavior = get_behavior_from_id(behaviorId);

    // check if remote coin id has already been seen
    for (int i = 0; i < MAX_REMOTE_COIN_IDS; i++) {
        if (remoteCoinIds[i] == remoteCoinId) {
            // we already saw this coin!
            goto SANITY_CHECK_COINS;
        }
    }
    // cache the seen id
    remoteCoinIds[onRemoteCoinId] = remoteCoinId;
    onRemoteCoinId = (onRemoteCoinId + 1) % MAX_REMOTE_COIN_IDS;

    // make sure it's valid
    if (behavior == NULL) { goto SANITY_CHECK_COINS; }

    // find the coin
    struct Object* coin = find_nearest_coin(behavior, pos, coinValue, 1000);
    if (coin == NULL) { goto SANITY_CHECK_COINS; }

    // destroy coin
    coin->oInteractStatus = INT_STATUS_INTERACTED;

    // add to local mario's coin count
    gMarioStates[0].numCoins += coinValue;

    // check for 100-coin star
    extern s16 gCurrCourseNum;
    if (COURSE_IS_MAIN_COURSE(gCurrCourseNum)
        && gMarioStates[0].numCoins - coin->oDamageOrCoinValue < 100
        && gMarioStates[0].numCoins >= 100) {
        bhv_spawn_star_no_level_exit(gMarioStates[1].marioObj, 6);
    }

    return;

SANITY_CHECK_COINS:;
    // make sure we're at least at the same coin count
    s16 oldCoinCount = gMarioStates[0].numCoins;
    gMarioStates[0].numCoins = max(numCoins, gMarioStates[0].numCoins);

    // check for 100-coin star
    if (COURSE_IS_MAIN_COURSE(gCurrCourseNum)
        && oldCoinCount < 100
        && gMarioStates[0].numCoins >= 100) {
        bhv_spawn_star_no_level_exit(gMarioStates[1].marioObj, 6);
    }
}
