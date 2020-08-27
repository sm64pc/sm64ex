// scuttlebug.c.inc

struct ObjectHitbox sScuttlebugHitbox = {
    /* interactType: */ INTERACT_BOUNCE_TOP,
    /* downOffset: */ 0,
    /* damageOrCoinValue: */ 1,
    /* health: */ 1,
    /* numLootCoins: */ 3,
    /* radius: */ 130,
    /* height: */ 70,
    /* hurtboxRadius: */ 90,
    /* hurtboxHeight: */ 60,
};

s32 update_angle_from_move_flags(s32 *angle) {
    if (o->oMoveFlags & OBJ_MOVE_HIT_WALL) {
        *angle = o->oWallAngle;
        return 1;
    } else if (o->oMoveFlags & OBJ_MOVE_HIT_EDGE) {
        *angle = o->oMoveAngleYaw + 0x8000;
        return -1;
    }
    return 0;
}

void bhv_scuttlebug_loop(void) {
    if (!network_sync_object_initialized(o)) {
        network_init_object(o, 4000.0f);
        network_init_object_field(o, &o->oFlags);
        network_init_object_field(o, &o->oForwardVel);
        network_init_object_field(o, &o->oHomeX);
        network_init_object_field(o, &o->oHomeY);
        network_init_object_field(o, &o->oHomeZ);
        network_init_object_field(o, &o->oInteractStatus);
        network_init_object_field(o, &o->oScuttlebugUnkF4);
    }

    struct Object* player = nearest_player_to_object(o);
    int angleToPlayer = obj_angle_to_object(o, player);

    UNUSED s32 unused;
    f32 sp18;
    cur_obj_update_floor_and_walls();
    if (o->oSubAction != 0
        && cur_obj_set_hitbox_and_die_if_attacked(&sScuttlebugHitbox, SOUND_OBJ_DYING_ENEMY1,
                                              o->oScuttlebugUnkF4))
        o->oSubAction = 3;
    if (o->oSubAction != 1)
        o->oScuttlebugUnkF8 = 0;
    switch (o->oSubAction) {
        case 0:
            if (o->oMoveFlags & OBJ_MOVE_LANDED)
                cur_obj_play_sound_2(SOUND_OBJ_GOOMBA_ALERT);
            if (o->oMoveFlags & OBJ_MOVE_MASK_ON_GROUND) {
                o->oHomeX = o->oPosX;
                o->oHomeY = o->oPosY;
                o->oHomeZ = o->oPosZ;
                o->oSubAction++;
            }
            break;
        case 1:
            o->oForwardVel = 5.0f;
            if (cur_obj_lateral_dist_from_obj_to_home(player) > 1000.0f)
                o->oAngleToMario = angleToPlayer;
            else {
                if (o->oScuttlebugUnkF8 == 0) {
                    o->oScuttlebugUnkFC = 0;
                    o->oAngleToMario = obj_angle_to_object(o, player);
                    if (abs_angle_diff(angleToPlayer, o->oMoveAngleYaw) < 0x800) {
                        o->oScuttlebugUnkF8 = 1;
                        o->oVelY = 20.0f;
                        cur_obj_play_sound_2(SOUND_OBJ2_SCUTTLEBUG_ALERT);
                    }
                } else if (o->oScuttlebugUnkF8 == 1) {
                    o->oForwardVel = 15.0f;
                    o->oScuttlebugUnkFC++;
                    if (o->oScuttlebugUnkFC > 50)
                        o->oScuttlebugUnkF8 = 0;
                }
            }
            if (update_angle_from_move_flags(&angleToPlayer))
                o->oSubAction = 2;
            cur_obj_rotate_yaw_toward(angleToPlayer, 0x200);
            break;
        case 2:
            o->oForwardVel = 5.0f;
            if ((s16) o->oMoveAngleYaw == (s16) o->oAngleToMario)
                o->oSubAction = 1;
            if (o->oPosY - o->oHomeY < -200.0f)
                obj_mark_for_deletion(o);
            cur_obj_rotate_yaw_toward(o->oAngleToMario, 0x400);
            break;
        case 3:
            o->oFlags &= ~8;
            o->oForwardVel = -10.0f;
            o->oVelY = 30.0f;
            cur_obj_play_sound_2(SOUND_OBJ2_SCUTTLEBUG_ALERT);
            o->oSubAction++;
            break;
        case 4:
            o->oForwardVel = -10.0f;
            if (o->oMoveFlags & OBJ_MOVE_LANDED) {
                o->oSubAction++;
                o->oVelY = 0.0f;
                o->oScuttlebugUnkFC = 0;
                o->oFlags |= 8;
                o->oInteractStatus = 0;
            }
            break;
        case 5:
            o->oForwardVel = 2.0f;
            o->oScuttlebugUnkFC++;
            if (o->oScuttlebugUnkFC > 30)
                o->oSubAction = 0;
            break;
    }
    if (o->oForwardVel < 10.0f)
        sp18 = 1.0f;
    else
        sp18 = 3.0f;
    cur_obj_init_animation_with_accel_and_sound(0, sp18);
    if (o->oMoveFlags & OBJ_MOVE_MASK_ON_GROUND)
        set_obj_anim_with_accel_and_sound(1, 23, SOUND_OBJ2_SCUTTLEBUG_WALK);
    if (o->parentObj != o) {
        if (obj_is_hidden(o))
            obj_mark_for_deletion(o);
        if (o->activeFlags == ACTIVE_FLAG_DEACTIVATED) {
            o->parentObj->oScuttlebugSpawnerUnk88 = 1;
            network_send_object(o->parentObj);
        }
    }
    cur_obj_move_standard(-50);
}

void bhv_scuttlebug_spawn_loop(void) {
    if (!network_sync_object_initialized(o)) {
        network_init_object(o, SYNC_DISTANCE_ONLY_EVENTS);
        network_init_object_field(o, &o->oAction);
        network_init_object_field(o, &o->oTimer);
        network_init_object_field(o, &o->oScuttlebugSpawnerUnkF4);
        network_init_object_field(o, &o->oScuttlebugSpawnerUnk88);
    }

    struct MarioState* marioState = nearest_mario_state_to_object(o);
    if (marioState->playerIndex != 0) { return; }

    struct Object* player = marioState->marioObj;
    int distanceToPlayer = dist_between_objects(o, player);

    struct Object *scuttlebug;
    if (o->oAction == 0) {
        if (o->oTimer > 30 && 500.0f < distanceToPlayer && distanceToPlayer < 1500.0f) {
            cur_obj_play_sound_2(SOUND_OBJ2_SCUTTLEBUG_ALERT);
            scuttlebug = spawn_object(o, MODEL_SCUTTLEBUG, bhvScuttlebug);
            scuttlebug->oScuttlebugUnkF4 = o->oScuttlebugSpawnerUnkF4;
            scuttlebug->oForwardVel = 30.0f;
            scuttlebug->oVelY = 80.0f;

            network_set_sync_id(scuttlebug);
            struct Object* spawn_objects[] = { scuttlebug };
            u32 models[] = { MODEL_SCUTTLEBUG };
            network_send_spawn_objects(spawn_objects, models, 1);

            o->oAction++;
            o->oScuttlebugUnkF4 = 1;
            network_send_object(o);
        }
    } else if (o->oScuttlebugSpawnerUnk88 != 0) {
        o->oScuttlebugSpawnerUnk88 = 0;
        o->oAction = 0;
        network_send_object(o);
    }
}
