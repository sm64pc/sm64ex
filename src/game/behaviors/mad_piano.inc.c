
static struct ObjectHitbox sMadPianoHitbox = {
    /* interactType:      */ INTERACT_MR_BLIZZARD,
    /* downOffset:        */ 0,
    /* damageOrCoinValue: */ 3,
    /* health:            */ 99,
    /* numLootCoins:      */ 0,
    /* radius:            */ 200,
    /* height:            */ 150,
    /* hurtboxRadius:     */ 200,
    /* hurtboxHeight:     */ 150,
};

static void mad_piano_act_wait(void) {
    cur_obj_init_animation_with_sound(0);

    struct MarioState* marioState = nearest_mario_state_to_object(o);
    struct Object* player = marioState->marioObj;
    int distanceToPlayer = dist_between_objects(o, player);

    if (distanceToPlayer < 500.0f) {
        if (o->oTimer > 20) {
            if (marioState->forwardVel > 10.0f) {
                o->oAction = MAD_PIANO_ACT_ATTACK;
                cur_obj_become_tangible();
            }
        }
    } else {
        o->oTimer = 0;
    }

    cur_obj_push_mario_away_from_cylinder(280.0f, 150.0f);
}

static void mad_piano_act_attack(void) {
    cur_obj_update_floor_and_walls();
    cur_obj_init_animation_with_sound(1);
    cur_obj_play_sound_at_anim_range(0, 0, SOUND_OBJ_MAD_PIANO_CHOMPING);

    struct MarioState* marioState = nearest_mario_state_to_object(o);
    struct Object* player = marioState->marioObj;
    int distanceToPlayer = dist_between_objects(o, player);
    int angleToPlayer = obj_angle_to_object(o, player);

    if (distanceToPlayer < 500.0f) {
        o->oTimer = 0;
    }

    if (o->oTimer > 80 && cur_obj_check_if_near_animation_end()) {
        o->oAction = MAD_PIANO_ACT_WAIT;
        o->oForwardVel = 0.0f;
        cur_obj_become_intangible();
    } else {
        f32 dx = o->oPosX - o->oHomeX;
        f32 dz = o->oPosZ - o->oHomeZ;
        f32 distToHome = sqrtf(dx * dx + dz * dz);

        if (distToHome > 400.0f) {
            distToHome = 400.0f / distToHome;
            o->oPosX = o->oHomeX + dx * distToHome;
            o->oPosZ = o->oHomeZ + dz * distToHome;
        }

        cur_obj_rotate_yaw_toward(angleToPlayer, 400);
        o->oForwardVel = 5.0f;
    }

    obj_check_attacks(&sMadPianoHitbox, o->oAction);
    cur_obj_move_standard(78);
}

void bhv_mad_piano_update(void) {
    if (!network_sync_object_initialized(o)) {
        network_init_object(o, 4000.0f);
    }

    if (!(o->activeFlags & ACTIVE_FLAG_IN_DIFFERENT_ROOM)) {
        o->oFaceAngleYaw = o->oMoveAngleYaw - 0x4000;

        switch (o->oAction) {
            case MAD_PIANO_ACT_WAIT:
                mad_piano_act_wait();
                break;
            case MAD_PIANO_ACT_ATTACK:
                mad_piano_act_attack();
                break;
        }
    }
}
