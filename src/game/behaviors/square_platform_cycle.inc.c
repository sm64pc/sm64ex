// square_platform_cycle.c.inc

void bhv_squarish_path_parent_init(void) {
    o->header.gfx.node.flags |= GRAPH_RENDER_INVISIBLE;
    cur_obj_become_intangible();

    static s16 radius = (614 / 2);
    o->oPosX += radius;
    o->oPosZ += radius;

    struct SyncObject* so = network_init_object(o, 2000.0f);
    so->hasStandardFields = FALSE;
    so->maxUpdateRate = 5.0f;

    for (int i = 0; i < 2; i++) {
        s16 action = (i == 0) ? 1 : 3;
        s16 offset = (i == 0) ? (-radius) : (radius);
        struct Object* square = spawn_object(o, MODEL_BITDW_SQUARE_PLATFORM, bhvSquarishPathMoving);
        square->oPosX = o->oPosX + offset;
        square->oPosY = o->oPosY;
        square->oPosZ = o->oPosZ + offset;
        square->oAction = action;
        network_init_object_field(o, &square->oMoveAngleYaw);
        network_init_object_field(o, &square->oForwardVel);
        network_init_object_field(o, &square->oPosX);
        network_init_object_field(o, &square->oPosY);
        network_init_object_field(o, &square->oPosZ);
        network_init_object_field(o, &square->oVelX);
        network_init_object_field(o, &square->oVelY);
        network_init_object_field(o, &square->oVelZ);
        network_init_object_field(o, &square->oAction);
        network_init_object_field(o, &square->oPrevAction);
        network_init_object_field(o, &square->oTimer);
    }
}

void bhv_squarish_path_parent_loop(void) { }

s32 square_plat_set_yaw_until_timer(u16 yaw, s32 a) {
    o->oMoveAngleYaw = yaw;
    if (a < o->oTimer)
        return 1;
    else
        return 0;
}

void bhv_squarish_path_moving_loop(void) {
    o->oForwardVel = 10.0f;
    switch (o->oAction) {
        case 0:
            o->oAction = (o->oBehParams2ndByte & 3) + 1;
            break;
        case 1:
            if (square_plat_set_yaw_until_timer(0, 60))
                o->oAction++;
            break;
        case 2:
            if (square_plat_set_yaw_until_timer(0x4000, 60))
                o->oAction++;
            break;
        case 3:
            if (square_plat_set_yaw_until_timer(0x8000, 60))
                o->oAction++;
            break;
        case 4:
            if (square_plat_set_yaw_until_timer(0xc000, 60))
                o->oAction = 1;
            break;
        default:
            break;
    }
    cur_obj_move_using_fvel_and_gravity();
    load_object_collision_model();
}
