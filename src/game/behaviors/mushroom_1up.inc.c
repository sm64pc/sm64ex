// mushroom_1up.c.inc

void bhv_1up_interact(void) {
    UNUSED s32 sp1C;

    struct MarioState* marioState = nearest_mario_state_to_object(o);
    if (marioState->playerIndex == 0 && obj_check_if_collided_with_object(o, marioState->marioObj) == 1) {
        play_sound(SOUND_GENERAL_COLLECT_1UP, gDefaultSoundArgs);
        marioState->numLives++;
        o->activeFlags = ACTIVE_FLAG_DEACTIVATED;
    }
}

void bhv_1up_common_init(void) {
    o->oMoveAnglePitch = -0x4000;
    o->oGravity = 3.0f;
    o->oFriction = 1.0f;
    o->oBuoyancy = 1.0f;
}

void bhv_1up_init(void) {
    bhv_1up_common_init();
    if (o->oBehParams2ndByte == 1) {
        if ((save_file_get_flags() & 0x50) == 0)
            o->activeFlags = ACTIVE_FLAG_DEACTIVATED;
    } else if (o->oBehParams2ndByte == 2) {
        if ((save_file_get_flags() & 0xa0) == 0)
            o->activeFlags = ACTIVE_FLAG_DEACTIVATED;
    }
}

void one_up_loop_in_air(void) {
    if (o->oTimer < 5) {
        o->oVelY = 40.0f;
    } else {
        o->oAngleVelPitch = -0x1000;
        o->oMoveAnglePitch += o->oAngleVelPitch;
        o->oVelY = coss(o->oMoveAnglePitch) * 30.0f + 2.0f;
        o->oForwardVel = -sins(o->oMoveAnglePitch) * 30.0f;
    }
}

void pole_1up_move_towards_mario(void) {
    struct Object* player = nearest_player_to_object(o);
    f32 sp34 = player->header.gfx.pos[0] - o->oPosX;
    f32 sp30 = player->header.gfx.pos[1] + 120.0f - o->oPosY;
    f32 sp2C = player->header.gfx.pos[2] - o->oPosZ;
    s16 sp2A = atan2s(sqrtf(sqr(sp34) + sqr(sp2C)), sp30);

    obj_turn_toward_object(o, player, 16, 0x1000);
    o->oMoveAnglePitch = approach_s16_symmetric(o->oMoveAnglePitch, sp2A, 0x1000);
    o->oVelY = sins(o->oMoveAnglePitch) * 30.0f;
    o->oForwardVel = coss(o->oMoveAnglePitch) * 30.0f;
    bhv_1up_interact();
}

void one_up_move_away_from_mario(s16 sp1A) {
    struct Object* player = nearest_player_to_object(o);
    int angleToPlayer = obj_angle_to_object(o, player);

    o->oForwardVel = 8.0f;
    o->oMoveAngleYaw = angleToPlayer + 0x8000;
    bhv_1up_interact();
    if (sp1A & 0x02)
        o->oAction = 2;

    if (!is_point_within_radius_of_mario(o->oPosX, o->oPosY, o->oPosZ, 3000))
        o->oAction = 2;
}

void bhv_1up_walking_loop(void) {
    object_step();

    switch (o->oAction) {
        case 0:
            if (o->oTimer >= 18)
                spawn_object(o, MODEL_NONE, bhvSparkleSpawn);

            if (o->oTimer == 0)
                play_sound(SOUND_GENERAL2_1UP_APPEAR, gDefaultSoundArgs);

            one_up_loop_in_air();

            if (o->oTimer == 37) {
                cur_obj_become_tangible();
                o->oAction = 1;
                o->oForwardVel = 2.0f;
            }
            break;

        case 1:
            if (o->oTimer > 300)
                o->oAction = 2;

            bhv_1up_interact();
            break;

        case 2:
            obj_flicker_and_disappear(o, 30);
            bhv_1up_interact();
            break;
    }

    set_object_visibility(o, 3000);
}

void bhv_1up_running_away_loop(void) {
    s16 sp26;

    sp26 = object_step();
    switch (o->oAction) {
        case 0:
            if (o->oTimer >= 18)
                spawn_object(o, MODEL_NONE, bhvSparkleSpawn);

            if (o->oTimer == 0)
                play_sound(SOUND_GENERAL2_1UP_APPEAR, gDefaultSoundArgs);

            one_up_loop_in_air();

            if (o->oTimer == 37) {
                cur_obj_become_tangible();
                o->oAction = 1;
                o->oForwardVel = 8.0f;
            }
            break;

        case 1:
            spawn_object(o, MODEL_NONE, bhvSparkleSpawn);
            one_up_move_away_from_mario(sp26);
            break;

        case 2:
            obj_flicker_and_disappear(o, 30);
            bhv_1up_interact();
            break;
    }

    set_object_visibility(o, 3000);
}

void sliding_1up_move(void) {
    s16 sp1E;

    sp1E = object_step();
    if (sp1E & 0x01) {
        o->oForwardVel += 25.0f;
        o->oVelY = 0;
    } else {
        o->oForwardVel *= 0.98;
    }

    if (o->oForwardVel > 40.0)
        o->oForwardVel = 40.0f;

    if (!is_point_within_radius_of_mario(o->oPosX, o->oPosY, o->oPosZ, 5000))
        o->oAction = 2;
}

void bhv_1up_sliding_loop(void) {
    switch (o->oAction) {
        case 0:
            set_object_visibility(o, 3000);
            if (is_point_within_radius_of_mario(o->oPosX, o->oPosY, o->oPosZ, 1000))
                o->oAction = 1;
            break;

        case 1:
            sliding_1up_move();
            break;

        case 2:
            obj_flicker_and_disappear(o, 30);
            bhv_1up_interact();
            break;
    }

    bhv_1up_interact();
    spawn_object(o, MODEL_NONE, bhvSparkleSpawn);
}

void bhv_1up_loop(void) {
    bhv_1up_interact();
    set_object_visibility(o, 3000);
}

void bhv_1up_jump_on_approach_loop(void) {
    s16 sp26;

    switch (o->oAction) {
        case 0:
            if (is_point_within_radius_of_mario(o->oPosX, o->oPosY, o->oPosZ, 1000)) {
                o->oVelY = 40.0f;
                o->oAction = 1;
            }
            break;

        case 1:
            sp26 = object_step();
            one_up_move_away_from_mario(sp26);
            spawn_object(o, MODEL_NONE, bhvSparkleSpawn);
            break;

        case 2:
            sp26 = object_step();
            bhv_1up_interact();
            obj_flicker_and_disappear(o, 30);
            break;
    }

    set_object_visibility(o, 3000);
}

void bhv_1up_hidden_loop(void) {
    if (!network_sync_object_initialized(o)) {
        network_init_object(o, SYNC_DISTANCE_ONLY_EVENTS);
        network_init_object_field(o, &o->oVelY);
        network_init_object_field(o, &o->oAction);
        network_init_object_field(o, &o->header.gfx.node.flags);
    }
    s16 sp26;
    switch (o->oAction) {
        case 0:
            o->header.gfx.node.flags |= GRAPH_RENDER_INVISIBLE;
            if (o->o1UpHiddenUnkF4 == o->oBehParams2ndByte) {
                o->oVelY = 40.0f;
                o->oAction = 3;
                o->header.gfx.node.flags &= ~GRAPH_RENDER_INVISIBLE;
                play_sound(SOUND_GENERAL2_1UP_APPEAR, gDefaultSoundArgs);
                network_send_object(o);
            }
            break;

        case 1:
            sp26 = object_step();
            one_up_move_away_from_mario(sp26);
            spawn_object(o, MODEL_NONE, bhvSparkleSpawn);
            break;

        case 2:
            sp26 = object_step();
            bhv_1up_interact();
            obj_flicker_and_disappear(o, 30);
            break;

        case 3:
            sp26 = object_step();
            if (o->oTimer >= 18)
                spawn_object(o, MODEL_NONE, bhvSparkleSpawn);

            one_up_loop_in_air();

            if (o->oTimer == 37) {
                cur_obj_become_tangible();
                o->oAction = 1;
                o->oForwardVel = 8.0f;
            }
            break;
    }
}

void bhv_1up_hidden_trigger_loop(void) {
    if (!network_sync_object_initialized(o)) {
        network_init_object(o, SYNC_DISTANCE_ONLY_EVENTS);
        network_init_object_field(o, &o->o1UpForceSpawn);
    }
    struct Object* player = nearest_player_to_object(o);
    struct Object *sp1C;
    if (o->o1UpForceSpawn || obj_check_if_collided_with_object(o, player) == 1) {
        sp1C = cur_obj_nearest_object_with_behavior(bhvHidden1up);
        if (sp1C != NULL)
            sp1C->o1UpHiddenUnkF4++;
        o->o1UpForceSpawn = TRUE;
        network_send_object(o);
        o->activeFlags = ACTIVE_FLAG_DEACTIVATED;
    }
}

void bhv_1up_hidden_in_pole_loop(void) {
    if (!network_sync_object_initialized(o)) {
        network_init_object(o, SYNC_DISTANCE_ONLY_EVENTS);
        network_init_object_field(o, &o->oVelY);
        network_init_object_field(o, &o->oAction);
        network_init_object_field(o, &o->header.gfx.node.flags);
    }
    UNUSED s16 sp26;
    switch (o->oAction) {
        case 0:
            o->header.gfx.node.flags |= GRAPH_RENDER_INVISIBLE;
            if (o->o1UpHiddenUnkF4 == o->oBehParams2ndByte) {
                o->oVelY = 40.0f;
                o->oAction = 3;
                o->header.gfx.node.flags &= ~GRAPH_RENDER_INVISIBLE;
                play_sound(SOUND_GENERAL2_1UP_APPEAR, gDefaultSoundArgs);
                network_send_object(o);
            }
            break;

        case 1:
            pole_1up_move_towards_mario();
            sp26 = object_step();
            break;

        case 3:
            sp26 = object_step();
            if (o->oTimer >= 18)
                spawn_object(o, MODEL_NONE, bhvSparkleSpawn);

            one_up_loop_in_air();

            if (o->oTimer == 37) {
                cur_obj_become_tangible();
                o->oAction = 1;
                o->oForwardVel = 10.0f;
            }
            break;
    }
}

void bhv_1up_hidden_in_pole_trigger_loop(void) {
    if (!network_sync_object_initialized(o)) {
        network_init_object(o, SYNC_DISTANCE_ONLY_EVENTS);
        network_init_object_field(o, &o->o1UpForceSpawn);
    }
    struct Object *sp1C;

    struct Object* player = nearest_player_to_object(o);
    if (o->o1UpForceSpawn || obj_check_if_collided_with_object(o, player) == 1) {
        sp1C = cur_obj_nearest_object_with_behavior(bhvHidden1upInPole);
        if (sp1C != NULL) {
            sp1C->o1UpHiddenUnkF4++;
        }
        o->o1UpForceSpawn = TRUE;
        network_send_object(o);
        o->activeFlags = ACTIVE_FLAG_DEACTIVATED;
    }
}

void bhv_1up_hidden_in_pole_spawner_loop(void) {
    if (!network_sync_object_initialized(o)) {
        network_init_object(o, SYNC_DISTANCE_ONLY_EVENTS);
        network_init_object_field(o, &o->o1UpForceSpawn);
    }

    s8 sp2F;

    if (o->o1UpForceSpawn || is_point_within_radius_of_mario(o->oPosX, o->oPosY, o->oPosZ, 700)) {
        spawn_object_relative(2, 0, 50, 0, o, MODEL_1UP, bhvHidden1upInPole);
        for (sp2F = 0; sp2F < 2; sp2F++) {
            spawn_object_relative(0, 0, sp2F * -200, 0, o, MODEL_NONE, bhvHidden1upInPoleTrigger);
        }
        o->o1UpForceSpawn = TRUE;
        network_send_object(o);
        o->activeFlags = ACTIVE_FLAG_DEACTIVATED;
    }
}
