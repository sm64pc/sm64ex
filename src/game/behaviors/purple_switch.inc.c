/**
 * Behavior for bhvFloorSwitchHardcodedModel, bhvFloorSwitchGrills, and
 * bhvFloorSwitchAnimatesObject.
 *
 * This controls the purple switches that Mario can step on to affect parts of
 * the environment.
 */

void bhv_purple_switch_loop(void) {
    if (!network_sync_object_initialized(o)) {
        network_init_object(o, SYNC_DISTANCE_ONLY_EVENTS);
        network_init_object_field(o, &o->oAction);
        network_init_object_field(o, &o->oTimer);
    }

    u8 anyPlayerOnPlatform = FALSE;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!is_player_active(&gMarioStates[i])) { continue; }
        if (gMarioStates[i].marioObj->platform == o) {
            anyPlayerOnPlatform = TRUE;
            break;
        }
    }

    UNUSED s32 unused;
    switch (o->oAction) {
        /**
         * Set the switch's model and scale. If Mario is standing near the
         * switch's middle section, transition to the pressed state.
         */
        case PURPLE_SWITCH_IDLE:
            cur_obj_set_model(MODEL_PURPLE_SWITCH);
            cur_obj_scale(1.5f);
            if (gMarioObject->platform == o && !(gMarioStates->action & MARIO_UNKNOWN_13)) {
                if (lateral_dist_between_objects(o, gMarioObject) < 127.5) {
                    o->oAction = PURPLE_SWITCH_PRESSED;
                    network_send_object(o);
                }
            }
            break;
        /**
         * Collapse the switch downward, play a sound, and shake the screen.
         * Immediately transition to the ticking state.
         */
        case PURPLE_SWITCH_PRESSED:
            cur_obj_scale_over_time(2, 3, 1.5f, 0.2f);
            if (o->oTimer == 3) {
                cur_obj_play_sound_2(SOUND_GENERAL2_PURPLE_SWITCH);
                o->oAction = PURPLE_SWITCH_TICKING;
                cur_obj_shake_screen(SHAKE_POS_SMALL);
                queue_rumble_data_object(o, 5, 80);
            }
            break;
        /**
         * Play a continuous ticking sound that gets faster when time is almost
         * up. When time is up, move to a waiting-while-pressed state.
         */
        case PURPLE_SWITCH_TICKING:
            if (o->oBehParams2ndByte != 0) {
                if (o->oBehParams2ndByte == 1 && !anyPlayerOnPlatform) {
                    o->oAction++;
                } else {
                    if (o->oTimer < 360) {
                        play_sound(SOUND_GENERAL2_SWITCH_TICK_FAST, gDefaultSoundArgs);
                    } else {
                        play_sound(SOUND_GENERAL2_SWITCH_TICK_SLOW, gDefaultSoundArgs);
                    }
                    if (o->oTimer > 400) {
                        o->oAction = PURPLE_SWITCH_WAIT_FOR_MARIO_TO_GET_OFF;
                    }
                }
            }
            break;
        /**
         * Make the switch look unpressed again, and transition back to the
         * idle state.
         */
        case PURPLE_SWITCH_UNPRESSED:
            cur_obj_scale_over_time(2, 3, 0.2f, 1.5f);
            if (o->oTimer == 3) {
                o->oAction = PURPLE_SWITCH_IDLE;
                network_send_object(o);
            }
            break;
        /**
         * Mario is standing on the switch, but time has expired. Wait for
         * him to get off the switch, and when he does so, transition to the
         * unpressed state.
         */
        case PURPLE_SWITCH_WAIT_FOR_MARIO_TO_GET_OFF:
            if (!anyPlayerOnPlatform) {
                o->oAction = PURPLE_SWITCH_UNPRESSED;
            }
            break;
    }
}
