// floating_platform.c.inc

f32 floating_platform_find_home_y(void) {
    struct Surface *sp24;
    f32 sp20;
    f32 sp1C;

    sp20 = find_water_level(o->oPosX, o->oPosZ);
    sp1C = find_floor(o->oPosX, o->oPosY, o->oPosZ, &sp24);
    if (sp20 > sp1C + o->oFloatingPlatformUnkFC) {
        o->oFloatingPlatformUnkF4 = 0;
        return sp20 + o->oFloatingPlatformUnkFC;
    } else {
        o->oFloatingPlatformUnkF4 = 1;
        return sp1C + o->oFloatingPlatformUnkFC;
    }
}

void floating_platform_act_0(void) {

    f32 x = 0;
    f32 z = 0;
    u8 playersTouched = 0;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!is_player_active(&gMarioStates[i])) { continue; }
        if (gMarioStates[i].marioObj->platform == o) {
            x += gMarioStates[i].marioObj->oPosX;
            z += gMarioStates[i].marioObj->oPosZ;
            playersTouched++;
        }
    }

    if (playersTouched > 0) {
        x /= (f32)playersTouched;
        z /= (f32)playersTouched;

        s16 sp6 = (x - o->oPosX) * coss(-1*o->oMoveAngleYaw)
                  + (z - o->oPosZ) * sins(-1*o->oMoveAngleYaw);
        s16 sp4 = (z - o->oPosZ) * coss(-1*o->oMoveAngleYaw)
                  - (x - o->oPosX) * sins(-1*o->oMoveAngleYaw);

        o->oFaceAnglePitch = sp4 * 2;
        o->oFaceAngleRoll = -sp6 * 2;
        o->oVelY -= 1.0f;
        if (o->oVelY < 0.0f)
            o->oVelY = 0.0f;

        o->oFloatingPlatformUnkF8 += o->oVelY;
        if (o->oFloatingPlatformUnkF8 > 90.0f)
            o->oFloatingPlatformUnkF8 = 90.0f;
    } else {
        o->oFaceAnglePitch /= 2;
        o->oFaceAngleRoll /= 2;
        o->oFloatingPlatformUnkF8 -= 5.0;
        o->oVelY = 10.0f;
        if (o->oFloatingPlatformUnkF8 < 0.0f)
            o->oFloatingPlatformUnkF8 = 0.0f;
    }

    o->oPosY = o->oHomeY - 64.0f - o->oFloatingPlatformUnkF8 + sins(o->oFloatingPlatformUnk100 * 0x800) * 10.0f;
    o->oFloatingPlatformUnk100++;
    if (o->oFloatingPlatformUnk100 == 32)
        o->oFloatingPlatformUnk100 = 0;
}

void bhv_floating_platform_loop(void) {
    o->oHomeY = floating_platform_find_home_y();
    if (o->oFloatingPlatformUnkF4 == 0)
        o->oAction = 0;
    else
        o->oAction = 1;

    switch (o->oAction) {
        case 0:
            floating_platform_act_0();
            break;

        case 1:
            o->oPosY = o->oHomeY;
            break;
    }
}
