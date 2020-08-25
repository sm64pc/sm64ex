// bullet_bill.inc.c

// bullet bill smoke
void bhv_white_puff_smoke_init(void) {
    cur_obj_scale(random_float() * 2 + 2.0);
}

void bhv_bullet_bill_init(void) {
    o->oBulletBillInitialMoveYaw = o->oMoveAngleYaw;
    if (!network_sync_object_initialized(o)) {
        network_init_object(o, 4000.0f);
        network_init_object_field(o, &o->oFaceAnglePitch);
        network_init_object_field(o, &o->oFaceAngleRoll);
        network_init_object_field(o, &o->oForwardVel);
    }
}

void bullet_bill_act_0(void) {
    cur_obj_become_tangible();
    o->oForwardVel = 0.0f;
    o->oMoveAngleYaw = o->oBulletBillInitialMoveYaw;
    o->oFaceAnglePitch = 0;
    o->oFaceAngleRoll = 0;
    o->oMoveFlags = 0;
    cur_obj_set_pos_to_home();
    o->oAction = 1;
}

void bullet_bill_act_1(void) {
    struct Object* player = nearest_player_to_object(o);
    int distanceToPlayer = dist_between_objects(o, player);
    int angleToPlayer = obj_angle_to_object(o, player);

    s16 sp1E = abs_angle_diff(angleToPlayer, o->oMoveAngleYaw);
    if (sp1E < 0x2000 && 400.0f < distanceToPlayer && distanceToPlayer < 1500.0f)
        o->oAction = 2;
}

void bullet_bill_act_2(void) {
    struct Object* player = nearest_player_to_object(o);
    int distanceToPlayer = dist_between_objects(o, player);
    int angleToPlayer = obj_angle_to_object(o, player);

    if (o->oTimer < 40)
        o->oForwardVel = 3.0f;
    else if (o->oTimer < 50) {
        if (o->oTimer % 2)
            o->oForwardVel = 3.0f;
        else
            o->oForwardVel = -3.0f;
    } else {
        if (o->oTimer > 70)
            cur_obj_update_floor_and_walls();
        spawn_object(o, MODEL_SMOKE, bhvWhitePuffSmoke);
        o->oForwardVel = 30.0f;
        if (distanceToPlayer > 300.0f)
            cur_obj_rotate_yaw_toward(angleToPlayer, 0x100);
        if (o->oTimer == 50) {
            cur_obj_play_sound_2(SOUND_OBJ_POUNDING_CANNON);
            cur_obj_shake_screen(SHAKE_POS_SMALL);
        }
        if (o->oTimer > 150 || o->oMoveFlags & OBJ_MOVE_HIT_WALL) {
            o->oAction = 3;
            spawn_mist_particles();
        }
    }
}

void bullet_bill_act_3(void) {
    o->oAction = 0;
}

void bullet_bill_act_4(void) {
    if (o->oTimer == 0) {
        o->oForwardVel = -30.0f;
        cur_obj_become_intangible();
    }
    o->oFaceAnglePitch += 0x1000;
    o->oFaceAngleRoll += 0x1000;
    o->oPosY += 20.0f;
    if (o->oTimer > 90)
        o->oAction = 0;
}

void (*sBulletBillActions[])(void) = { bullet_bill_act_0, bullet_bill_act_1, bullet_bill_act_2,
                                       bullet_bill_act_3, bullet_bill_act_4 };

void bhv_bullet_bill_loop(void) {
    cur_obj_call_action_function(sBulletBillActions);
    if (cur_obj_check_interacted())
        o->oAction = 4;
}
