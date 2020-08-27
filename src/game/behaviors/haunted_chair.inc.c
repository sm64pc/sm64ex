// haunted_chair.inc.c

struct ObjectHitbox sHauntedChairHitbox = {
    /* interactType:      */ INTERACT_MR_BLIZZARD,
    /* downOffset:        */ 0,
    /* damageOrCoinValue: */ 2,
    /* health:            */ 0,
    /* numLootCoins:      */ 0,
    /* radius:            */ 50,
    /* height:            */ 50,
    /* hurtboxRadius:     */ 50,
    /* hurtboxHeight:     */ 50,
};

void bhv_haunted_chair_init(void) {
    struct Object *val04;
    f32 val00;

    val04 = cur_obj_find_nearest_object_with_behavior(bhvMadPiano, &val00);
    if (val04 != NULL && val00 < 300.0f) {
        o->parentObj = val04;
    } else {
        o->oHauntedChairUnkF4 = 1;
    }

    network_init_object(o, 4000.0f);
    network_init_object_field(o, &o->oFaceAnglePitch);
    network_init_object_field(o, &o->oFaceAngleRoll);
    network_init_object_field(o, &o->oFaceAngleYaw);
    network_init_object_field(o, &o->oHauntedChairUnk100);
    network_init_object_field(o, &o->oHauntedChairUnk104);
    network_init_object_field(o, &o->oHauntedChairUnkF4);
    network_init_object_field(o, &o->oHauntedChairUnkF8);
    network_init_object_field(o, &o->oHauntedChairUnkFC);
    network_init_object_field(o, &o->oMoveAnglePitch);
    network_init_object_field(o, &o->oMoveAngleYaw);
}

void haunted_chair_act_0(void) {
    s16 val0E;
    f32 val08;

    struct Object* player = nearest_player_to_object(o);
    int distanceToPlayer = dist_between_objects(o, player);

    if (o->parentObj != o) {
        if (o->oHauntedChairUnk104 == 0) {
            if (lateral_dist_between_objects(o, o->parentObj) < 250.0f) {
                val0E = obj_angle_to_object(o, o->parentObj) - o->oFaceAngleYaw + 0x2000;
                if (val0E & 0x4000) {
                    o->oHauntedChairUnk100 = &o->oFaceAngleRoll;
                    if (val0E > 0) {
                        o->oHauntedChairUnk104 = 0x4000;
                    } else {
                        o->oHauntedChairUnk104 = -0x4000;
                    }
                } else {
                    o->oHauntedChairUnk100 = &o->oFaceAnglePitch;
                    if (val0E < 0) {
                        o->oHauntedChairUnk104 = 0x5000;
                    } else {
                        o->oHauntedChairUnk104 = -0x4000;
                    }
                }

                if (o->oHauntedChairUnk104 < 0) {
                    o->oHauntedChairUnkF8 = -1500.0f;
                } else {
                    o->oHauntedChairUnkF8 = 1500.0f;
                }
            }
        } else {
            oscillate_toward(o->oHauntedChairUnk100, &o->oHauntedChairUnkF8, o->oHauntedChairUnk104,
                             4000.0f, 20.0f, 2.0f);
        }
    } else if (o->oHauntedChairUnkF4 != 0) {
        if (distanceToPlayer < 500.0f) {
            o->oHauntedChairUnkF4 -= 1;
        }
        o->oTimer = 0.0f;
    } else {
        if ((o->oTimer & 0x8) != 0) {
            if (o->oFaceAnglePitch < 0) {
                cur_obj_play_sound_2(SOUND_GENERAL_HAUNTED_CHAIR_MOVE);
                val08 = 4.0f;
            } else {
                val08 = -4.0f;
            }

            o->oHomeX -= val08;
            o->oHomeZ -= val08;

            o->oFaceAnglePitch = o->oFaceAngleRoll = (s32)(50.0f * val08);
            ;
        } else {
            o->oFaceAnglePitch = o->oFaceAngleRoll = 0;
        }

        if (o->oTimer > 30) {
            o->oAction = 1;
            o->oHauntedChairUnkF8 = 0.0f;
            o->oHauntedChairUnkFC = 200.0f;
            o->oHauntedChairUnkF4 = 40;
        }
    }

    cur_obj_push_mario_away_from_cylinder(80.0f, 120.0f);
}

void haunted_chair_act_1(void) {
    cur_obj_update_floor_and_walls();

    struct MarioState* marioState = nearest_mario_state_to_object(o);
    struct Object* player = marioState->marioObj;
    int angleToPlayer = obj_angle_to_object(o, player);

    if (o->oTimer < 70) {
        if (o->oTimer < 50) {
            o->oVelY = 6.0f;
        } else {
            o->oVelY = 0.0f;
        }

        o->oGravity = 0.0f;
        oscillate_toward(&o->oFaceAnglePitch, &o->oHauntedChairUnkF8, -4000, 200.0f, 20.0f, 2.0f);
        oscillate_toward(&o->oFaceAngleRoll, &o->oHauntedChairUnkFC, 0, 0.0f, 20.0f, 1.0f);
    } else {
        if (o->oHauntedChairUnkF4 != 0) {
            if (--o->oHauntedChairUnkF4 == 0) {
                cur_obj_play_sound_2(SOUND_GENERAL_HAUNTED_CHAIR);
                o->oMoveAnglePitch = obj_turn_pitch_toward_mario(marioState, 120.0f, 0);
                o->oMoveAngleYaw = angleToPlayer;
                obj_compute_vel_from_move_pitch(50.0f);
            } else if (o->oHauntedChairUnkF4 > 20) {
                if (gGlobalTimer % 4 == 0) {
                    cur_obj_play_sound_2(SOUND_GENERAL_SWISH_AIR_2);
                }
                o->oFaceAngleYaw += 0x2710;
            }
        } else if (o->oMoveFlags & (OBJ_MOVE_MASK_ON_GROUND | OBJ_MOVE_HIT_WALL)) {
            obj_die_if_health_non_positive();
        }
    }

    obj_check_attacks(&sHauntedChairHitbox, o->oAction);
    cur_obj_move_standard(78);
}

void bhv_haunted_chair_loop(void) {
    if (!(o->activeFlags & ACTIVE_FLAG_IN_DIFFERENT_ROOM)) {
        switch (o->oAction) {
            case 0:
                haunted_chair_act_0();
                break;
            case 1:
                haunted_chair_act_1();
                break;
        }
        cur_obj_spin_all_dimensions(30.0f, 30.0f);
    }
}
