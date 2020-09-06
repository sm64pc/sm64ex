/**
 * Behavior for bhvBbhTiltingTrapPlatform.
 * This is the tilting platform trap in the upper floor of BBH
 * that drops the player into the merry-go-round area.
 */

/**
 * Update function for bhvBbhTiltingTrapPlatform.
 */
void bhv_bbh_tilting_trap_platform_loop(void) {
    UNUSED s32 unused;

    if (!network_sync_object_initialized(o)) {
        network_init_object(o, 1000.0f);
        network_init_object_field(o, &o->oAngleVelPitch);
        network_init_object_field(o, &o->oFaceAnglePitch);
    }

    f32 x = 0;
    f32 y = 0;
    f32 z = 0;
    u8 playersTouched = 0;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!is_player_active(&gMarioStates[i])) { continue; }
        if (gMarioStates[i].marioObj->platform == o) {
            x += gMarioStates[i].marioObj->oPosX;
            y += gMarioStates[i].marioObj->oPosY;
            z += gMarioStates[i].marioObj->oPosZ;
            playersTouched++;
        }
    }

    int distanceToPlayer = 0;
    int angleToPlayer = 0;
    if (playersTouched > 0) {
        x /= (f32)playersTouched;
        y /= (f32)playersTouched;
        z /= (f32)playersTouched;
        distanceToPlayer = dist_between_object_and_point(o, x, y, z);
        angleToPlayer = obj_angle_to_point(o, x, z);
    }

    // US (and probably later) versions use oAction for the
    // if statement, while immediately setting it over here.
    // This was done so that Mario leaving or getting on the platform
    // resets oTimer to 0.
#ifndef VERSION_JP
    if (playersTouched > 0) {
        o->oAction = BBH_TILTING_TRAP_PLATFORM_ACT_MARIO_ON;
    } else {
        o->oAction = BBH_TILTING_TRAP_PLATFORM_ACT_MARIO_OFF;
    }

    if (o->oAction == BBH_TILTING_TRAP_PLATFORM_ACT_MARIO_ON) {
#else
    if (gMarioObject->platform == o) {
#endif
        o->oAngleVelPitch = (s32)(distanceToPlayer * coss(angleToPlayer));
        o->oFaceAnglePitch += o->oAngleVelPitch;
    } else
#ifndef VERSION_JP
        // In the US version, if the platform has tilted more than 3000 angle units
        // in less than 16 frames since Mario got on it, and he has stepped off,
        // this code will not run, so it will continue to rotate with the same
        // angular velocity for 16 more frames. This was probably done to make
        // the platform more dangerous. This code will not work correctly
        // without the oAction changes above, since oTimer will not ever
        // reset to 0 without them.
        if ((absi(o->oFaceAnglePitch) < 3000) || (o->oTimer >= 16))
#endif
    {
        // Make the platform return to the horizontal at a speed of
        // 200 angle units/frame, and clamp it to 0 if it's within 200 units of 0.
        o->oAngleVelPitch = 0;

        if ((s16) o->oFaceAnglePitch > 0) {
            if (o->oFaceAnglePitch < 200) {
                o->oFaceAnglePitch = 0;
            } else {
                o->oAngleVelPitch = -200;
            }
        } else {
            if (o->oFaceAnglePitch > -200) {
                o->oFaceAnglePitch = 0;
            } else {
                o->oAngleVelPitch = 200;
            }
        }
    }

    // Update angle
    o->oFaceAnglePitch += o->oAngleVelPitch;
}
