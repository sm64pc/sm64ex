#include <stdlib.h>
#include <time.h>
#include <PR/ultratypes.h>

#include "sm64.h"
#include "area.h"
#include "actors/common0.h"
#include "actors/common1.h"
#include "actors/group0.h"
#include "actors/group1.h"
#include "actors/group2.h"
#include "actors/group4.h"
#include "actors/group5.h"
#include "actors/group6.h"
#include "actors/group7.h"
#include "actors/group9.h"
#include "actors/group10.h"
#include "actors/group11.h"
#include "actors/group12.h"
#include "actors/group13.h"
#include "actors/group14.h"
#include "actors/group15.h"
#include "actors/group17.h"
#include "audio/data.h"
#include "audio/external.h"
#include "behavior_actions.h"
#include "behavior_data.h"
#include "camera.h"
#include "engine/behavior_script.h"
#include "engine/graph_node.h"
#include "engine/level_script.h"
#include "engine/math_util.h"
#include "engine/surface_collision.h"
#include "game_init.h"
#include "interaction.h"
#include "level_table.h"
#include "level_update.h"
#include "macros.h"
#include "main.h"
#include "mario.h"
#include "mario_actions_airborne.h"
#include "mario_actions_automatic.h"
#include "mario_actions_cutscene.h"
#include "mario_actions_moving.h"
#include "mario_actions_object.h"
#include "mario_actions_stationary.h"
#include "mario_actions_submerged.h"
#include "mario_cheats.h"
#include "mario_misc.h"
#include "mario_step.h"
#include "memory.h"
#include "model_ids.h"
#include "object_fields.h"
#include "object_helpers.h"
#include "object_list_processor.h"
#include "print.h"
#include "rendering_graph_node.h"
#include "save_file.h"
#include "seq_ids.h"
#include "sound_init.h"
#include "debug.h"
#include "thread6.h"
#include "pc/configfile.h"
#include "pc/cheats.h"
#ifdef R96
#include "sgi/utils/characters.h"
#endif // R96


#define SwiftSwim 42.0f

/*SwiftSwim Cheat*/
void cheats_swimming_speed(struct MarioState* m) {
    while (m->forwardVel < SwiftSwim && Cheats.EnableCheats == true && Cheats.Swim == true) {
        while (m->controller->buttonDown & A_BUTTON) {
            m->particleFlags |= PARTICLE_BUBBLE;
            m->forwardVel += 5.0f;
            break;
        }
        break;
    }
}

/*BLJAnywhere Cheat*/
void cheats_air_step(struct MarioState *m) {
    if (Cheats.BLJAnywhere > 0 && Cheats.EnableCheats == TRUE && m->action == ACT_LONG_JUMP
        && m->forwardVel < 1.0f && m->pos[1] - 50.0f < m->floorHeight) {
        if (Cheats.BLJAnywhere < 7) {
            if (m->controller->buttonPressed & A_BUTTON) {
                m->forwardVel -= (Cheats.BLJAnywhere - 1) * 2.5f;
                m->vel[1] = -50.0f;
            }
        } else if (m->controller->buttonDown & A_BUTTON) {
            m->forwardVel -= (Cheats.BLJAnywhere - 7) * 2.5f;
            m->vel[1] = -50.0f;
        }
    }
}

void cheats_long_jump(struct MarioState *m) {
    if (Cheats.BLJAnywhere >= 7 && Cheats.EnableCheats == true && m->forwardVel < 1.0f
        && (m->controller->buttonDown & A_BUTTON)) {
        set_jumping_action(m, ACT_LONG_JUMP, 0);
    }
}

/*Main cheat function*/
void cheats_mario_inputs(struct MarioState *m) {
    m->particleFlags = 0;
    m->collidedObjInteractTypes = m->marioObj->collidedObjInteractTypes;
    m->flags &= 0xFFFFFF;
    u32 r;

    while (Cheats.EnableCheats == true) {

        /*Spamba Per Level*/
        switch (gCurrLevelNum) {
            case LEVEL_CASTLE_GROUNDS:
                switch (SPL) {
                    case 0:
                        break;
                    case 1:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 160, 200, gCurrentObject, MODEL_BIRDS, bhvBird);
                            break;
                        }
                        break;
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            play_sound(SOUND_MENU_CAMERA_BUZZ, gDefaultSoundArgs);
                            break;
                        }
                        break;
                }
                break;
            case LEVEL_CASTLE_COURTYARD:
                switch (SPL) {
                    case 0:
                        break;
                    case 1:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 100, gCurrentObject, MODEL_MAD_PIANO,
                                                  bhvMadPiano);
                            break;
                        }
                        break;
                    case 2:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 150, gCurrentObject, MODEL_BOO, bhvBoo);
                            break;
                        }
                        break;
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            play_sound(SOUND_MENU_CAMERA_BUZZ, gDefaultSoundArgs);
                            break;
                        }
                        break;
                }
                break;
            case LEVEL_BITDW:
                switch (SPL) {
                    case 0:
                        break;
                    case 1:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 100, gCurrentObject, MODEL_BUBBA,
                                                  bhvBubba);
                            break;
                        }
                        break;
                    case 2:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 150, gCurrentObject, MODEL_ENEMY_LAKITU,
                                                  bhvEnemyLakitu);
                            break;
                        }
                        break;
                    case 3:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 300, 300, gCurrentObject, MODEL_SWOOP,
                                                  bhvSwoop);
                            break;
                        }
                        break;
                    case 4:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 200, gCurrentObject, MODEL_SNUFIT,
                                                  bhvSnufit);
                            break;
                        }
                        break;
                    case 5:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 10, 200, gCurrentObject, MODEL_DORRIE,
                                                  bhvDorrie);
                            break;
                        }
                        break;
                    case 6:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 100, gCurrentObject, MODEL_SCUTTLEBUG,
                                                  bhvScuttlebug);
                            break;
                        }
                        break;
                    case 7:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            play_sound(SOUND_MENU_CAMERA_BUZZ, gDefaultSoundArgs);
                            break;
                        }
                        break;
                }
                break;
            case LEVEL_BITFS:
            case LEVEL_LLL:
            case LEVEL_WMOTR:
                switch (SPL) {
                    case 0:
                        break;
                    case 1:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 100, gCurrentObject, MODEL_BULLY,
                                                  bhvSmallBully);
                            break;
                        }
                        break;
                    case 2:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 300, 300, gCurrentObject, MODEL_SWOOP,
                                                  bhvSwoop);
                            break;
                        }
                        break;
                    case 3:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 200, gCurrentObject, MODEL_SNUFIT,
                                                  bhvSnufit);
                            break;
                        }
                        break;
                    case 4:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 10, 200, gCurrentObject, MODEL_DORRIE,
                                                  bhvDorrie);
                            break;
                        }
                        break;
                    case 5:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 100, gCurrentObject, MODEL_SCUTTLEBUG,
                                                  bhvScuttlebug);
                            break;
                        }
                        break;
                    case 6:
                    case 7:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            play_sound(SOUND_MENU_CAMERA_BUZZ, gDefaultSoundArgs);
                            break;
                        }
                        break;
                }
                break;
            case LEVEL_BITS:
                switch (SPL) {
                    case 0:
                        break;
                    case 1:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 100, gCurrentObject,
                                                  MODEL_KOOPA_WITH_SHELL, bhvKoopa);
                            break;
                        }
                        break;
                    case 2:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 10, 200, gCurrentObject, MODEL_PIRANHA_PLANT,
                                                  bhvPiranhaPlant);
                            break;
                        }
                        break;
                    case 3:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 100, gCurrentObject, MODEL_WHOMP,
                                                  bhvSmallWhomp);
                            break;
                        }
                        break;
                    case 4:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 300, gCurrentObject, MODEL_CHAIN_CHOMP,
                                                  bhvChainChomp);
                            break;
                        }
                        break;
                    case 5:
                    case 6:
                    case 7:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            play_sound(SOUND_MENU_CAMERA_BUZZ, gDefaultSoundArgs);
                            break;
                        }
                        break;
                }
                break;
            case LEVEL_BOWSER_1:
            case LEVEL_BOWSER_2:
            case LEVEL_BOWSER_3:
                switch (SPL) {
                    case 0:
                        break;
                    case 1:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 200, 200, gCurrentObject, MODEL_BOWSER,
                                                  bhvBowser);
                            break;
                        }
                        break;
                    case 2:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 200, 200, gCurrentObject,
                                                  MODEL_BOWSER_BOMB_CHILD_OBJ, bhvBowserBomb);
                            break;
                        }
                        break;
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            play_sound(SOUND_MENU_CAMERA_BUZZ, gDefaultSoundArgs);
                            break;
                        }
                        break;
                }
                break;
            case LEVEL_BOB:
                switch (SPL) {
                    case 0:
                        break;
                    case 1:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 10, 300, gCurrentObject, MODEL_KING_BOBOMB,
                                                  bhvKingBobomb);
                            break;
                        }
                        break;
                    case 2:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 100, gCurrentObject,
                                                  MODEL_KOOPA_WITH_SHELL, bhvKoopa);
                            break;
                        }
                        break;
                    case 3:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 10, 200, gCurrentObject, MODEL_PIRANHA_PLANT,
                                                  bhvPiranhaPlant);
                            break;
                        }
                        break;
                    case 4:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 100, gCurrentObject, MODEL_WHOMP,
                                                  bhvSmallWhomp);
                            break;
                        }
                        break;
                    case 5:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 300, gCurrentObject, MODEL_CHAIN_CHOMP,
                                                  bhvChainChomp);
                            break;
                        }
                        break;
                    case 6:
                    case 7:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            play_sound(SOUND_MENU_CAMERA_BUZZ, gDefaultSoundArgs);
                            break;
                        }
                        break;
                }
                break;
            case LEVEL_BBH:
                switch (SPL) {
                    case 0:
                        break;
                    case 1:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 100, gCurrentObject, MODEL_MAD_PIANO,
                                                  bhvMadPiano);
                            break;
                        }
                        break;
                    case 2:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 150, gCurrentObject, MODEL_BOO, bhvBoo);
                            break;
                        }
                        break;
                    case 3:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 300, 300, gCurrentObject, MODEL_SWOOP,
                                                  bhvSwoop);
                            break;
                        }
                        break;
                    case 4:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 200, gCurrentObject, MODEL_SNUFIT,
                                                  bhvSnufit);
                            break;
                        }
                        break;
                    case 5:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 10, 200, gCurrentObject, MODEL_DORRIE,
                                                  bhvDorrie);
                            break;
                        }
                        break;
                    case 6:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 100, gCurrentObject, MODEL_SCUTTLEBUG,
                                                  bhvScuttlebug);
                            break;
                        }
                        break;
                    case 7:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            play_sound(SOUND_MENU_CAMERA_BUZZ, gDefaultSoundArgs);
                            break;
                        }
                        break;
                }
                break;
            case LEVEL_WF:
                switch (SPL) {
                    case 0:
                        break;
                    case 1:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 150, gCurrentObject, MODEL_THWOMP,
                                                  bhvThwomp);
                            break;
                        }
                        break;
                    case 2:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 300, 300, gCurrentObject, MODEL_BULLET_BILL,
                                                  bhvBulletBill);
                            break;
                        }
                        break;
                    case 3:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 200, gCurrentObject, MODEL_HEAVE_HO,
                                                  bhvHeaveHo);
                            break;
                        }
                        break;
                    case 4:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 100, gCurrentObject,
                                                  MODEL_KOOPA_WITH_SHELL, bhvKoopa);
                            break;
                        }
                        break;
                    case 5:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 10, 200, gCurrentObject, MODEL_PIRANHA_PLANT,
                                                  bhvPiranhaPlant);
                            break;
                        }
                        break;
                    case 6:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 100, gCurrentObject, MODEL_WHOMP,
                                                  bhvSmallWhomp);
                            break;
                        }
                        break;
                    case 7:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 300, gCurrentObject, MODEL_CHAIN_CHOMP,
                                                  bhvChainChomp);
                            break;
                        }
                        break;
                }
                break;
            case LEVEL_JRB:
            case LEVEL_DDD:
            case LEVEL_SA:
                switch (SPL) {
                    case 0:
                        break;
                    case 1:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 300, gCurrentObject, MODEL_CLAM_SHELL,
                                                  bhvClamShell);
                            break;
                        }
                        break;
                    case 2:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 100, gCurrentObject, MODEL_MANTA_RAY,
                                                  bhvMantaRay);
                            break;
                        }
                        break;
                    case 3:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 10, 100, gCurrentObject, MODEL_SUSHI,
                                                  bhvSushiShark);
                            break;
                        }
                        break;
                    case 4:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 300, 300, gCurrentObject, MODEL_UNAGI,
                                                  bhvUnagi);
                            break;
                        }
                        break;
                    case 5:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 100, gCurrentObject, MODEL_SKEETER,
                                                  bhvSkeeter);
                            break;
                        }
                        break;
                    case 6:
                    case 7:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            play_sound(SOUND_MENU_CAMERA_BUZZ, gDefaultSoundArgs);
                            break;
                        }
                        break;
                }
                break;
            case LEVEL_CCM:
            case LEVEL_SL:
                switch (SPL) {
                    case 0:
                        break;
                    case 1:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 100, gCurrentObject, MODEL_SPINDRIFT,
                                                  bhvSpindrift);
                            break;
                        }
                        break;
                    case 2:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 150, gCurrentObject, MODEL_BIG_CHILL_BULLY,
                                                  bhvBigChillBully);
                            break;
                        }
                        break;
                    case 3:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 200, gCurrentObject, MODEL_MONEYBAG,
                                                  bhvMoneybag);
                            break;
                        }
                        break;
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            play_sound(SOUND_MENU_CAMERA_BUZZ, gDefaultSoundArgs);
                            break;
                        }
                        break;
                }
                break;
            case LEVEL_HMC:
                switch (SPL) {
                    case 0:
                        break;
                    case 1:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 50, 100, gCurrentObject, MODEL_MONTY_MOLE,
                                                  bhvMontyMole);
                            break;
                        }
                        break;
                    case 2:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 200, gCurrentObject, MODEL_UKIKI,
                                                  bhvUkiki);
                            break;
                        }
                        break;
                    case 3:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 300, 300, gCurrentObject, MODEL_SWOOP,
                                                  bhvSwoop);
                            break;
                        }
                        break;
                    case 4:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 200, gCurrentObject, MODEL_SNUFIT,
                                                  bhvSnufit);
                            break;
                        }
                        break;
                    case 5:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 10, 200, gCurrentObject, MODEL_DORRIE,
                                                  bhvDorrie);
                            break;
                        }
                        break;
                    case 6:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 100, gCurrentObject, MODEL_SCUTTLEBUG,
                                                  bhvScuttlebug);
                            break;
                        }
                        break;
                    case 7:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            play_sound(SOUND_MENU_CAMERA_BUZZ, gDefaultSoundArgs);
                            break;
                        }
                        break;
                }
                break;
            case LEVEL_SSL:
                switch (SPL) {
                    case 0:
                        break;
                    case 1:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 200, gCurrentObject, MODEL_POKEY_HEAD,
                                                  bhvPokey);
                            break;
                        }
                        break;
                    case 2:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 300, 300, gCurrentObject, MODEL_KLEPTO,
                                                  bhvKlepto);
                            break;
                        }
                        break;
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            play_sound(SOUND_MENU_CAMERA_BUZZ, gDefaultSoundArgs);
                            break;
                        }
                        break;
                }
                break;
            case LEVEL_WDW:
                switch (SPL) {
                    case 0:
                        break;
                    case 1:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 150, gCurrentObject, MODEL_THWOMP,
                                                  bhvThwomp);
                            break;
                        }
                        break;
                    case 2:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 300, 300, gCurrentObject, MODEL_BULLET_BILL,
                                                  bhvBulletBill);
                            break;
                        }
                        break;
                    case 3:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 200, gCurrentObject, MODEL_HEAVE_HO,
                                                  bhvHeaveHo);
                            break;
                        }
                        break;
                    case 4:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 100, gCurrentObject, MODEL_SKEETER,
                                                  bhvSkeeter);
                            break;
                        }
                        break;
                    case 5:
                    case 6:
                    case 7:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            play_sound(SOUND_MENU_CAMERA_BUZZ, gDefaultSoundArgs);
                            break;
                        }
                        break;
                }
                break;
            case LEVEL_TTM:
                switch (SPL) {
                    case 0:
                        break;
                    case 1:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 100, gCurrentObject, MODEL_MONTY_MOLE,
                                                  bhvMontyMole);
                            break;
                        }
                        break;
                    case 2:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 200, gCurrentObject, MODEL_UKIKI,
                                                  bhvUkiki);
                            break;
                        }
                        break;
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            play_sound(SOUND_MENU_CAMERA_BUZZ, gDefaultSoundArgs);
                            break;
                        }
                        break;
                }
                break;
            case LEVEL_THI:
                switch (SPL) {
                    case 0:
                        break;
                    case 1:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 100, gCurrentObject, MODEL_BUBBA,
                                                  bhvBubba);
                            break;
                        }
                        break;
                    case 2:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 150, gCurrentObject, MODEL_ENEMY_LAKITU,
                                                  bhvEnemyLakitu);
                            break;
                        }
                        break;
                    case 3:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 100, gCurrentObject,
                                                  MODEL_KOOPA_WITH_SHELL, bhvKoopa);
                            break;
                        }
                        break;
                    case 4:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 10, 200, gCurrentObject, MODEL_PIRANHA_PLANT,
                                                  bhvPiranhaPlant);
                            break;
                        }
                        break;
                    case 5:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 100, gCurrentObject, MODEL_WHOMP,
                                                  bhvSmallWhomp);
                            break;
                        }
                        break;
                    case 6:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 300, gCurrentObject, MODEL_CHAIN_CHOMP,
                                                  bhvChainChomp);
                            break;
                        }
                        break;
                    case 7:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            play_sound(SOUND_MENU_CAMERA_BUZZ, gDefaultSoundArgs);
                            break;
                        }
                        break;
                }
                break;
            case LEVEL_TTC:
                switch (SPL) {
                    case 0:
                        break;
                    case 1:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 150, gCurrentObject, MODEL_THWOMP,
                                                  bhvThwomp);
                            break;
                        }
                        break;
                    case 2:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 300, 300, gCurrentObject, MODEL_BULLET_BILL,
                                                  bhvBulletBill);
                            break;
                        }
                        break;
                    case 3:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 200, gCurrentObject, MODEL_HEAVE_HO,
                                                  bhvHeaveHo);
                            break;
                        }
                        break;
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            play_sound(SOUND_MENU_CAMERA_BUZZ, gDefaultSoundArgs);
                            break;
                        }
                        break;
                }
                break;
            case LEVEL_RR:
                switch (SPL) {
                    case 0:
                        break;
                    case 1:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 100, gCurrentObject, MODEL_BUBBA,
                                                  bhvBubba);
                            break;
                        }
                        break;
                    case 2:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 150, gCurrentObject, MODEL_ENEMY_LAKITU,
                                                  bhvEnemyLakitu);
                            break;
                        }
                        break;
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            play_sound(SOUND_MENU_CAMERA_BUZZ, gDefaultSoundArgs);
                            break;
                        }
                        break;
                }
                break;
            case LEVEL_COTMC:
                switch (SPL) {
                    case 0:
                        break;
                    case 1:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 300, 300, gCurrentObject, MODEL_SWOOP,
                                                  bhvSwoop);
                            break;
                        }
                        break;
                    case 2:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 200, gCurrentObject, MODEL_SNUFIT,
                                                  bhvSnufit);
                            break;
                        }
                        break;
                    case 3:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 10, 200, gCurrentObject, MODEL_DORRIE,
                                                  bhvDorrie);
                            break;
                        }
                        break;
                    case 4:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            spawn_object_relative(0, 0, 100, 100, gCurrentObject, MODEL_SCUTTLEBUG,
                                                  bhvScuttlebug);
                            break;
                        }
                        break;
                    case 5:
                    case 6:
                    case 7:
                        if (m->controller->buttonDown & L_TRIG
                            && m->controller->buttonPressed & Z_TRIG) {
                            play_sound(SOUND_MENU_CAMERA_BUZZ, gDefaultSoundArgs);
                            break;
                        }
                        break;
                }
                break;
        }

        /*Drain JRB?*/
        f32 watLev;
        watLev -= 800;
        if (WAT_CON == 1) {
            watLev += WAT_LEV * 100;
        }
        if (WAT_CON == 1 && gCurrLevelNum == LEVEL_JRB) {
            gEnvironmentRegions[6] = approach_f32_symmetric(gEnvironmentRegions[6], watLev * 10, 10.0f);
            gEnvironmentRegions[12] = approach_f32_symmetric(gEnvironmentRegions[12], watLev * 10, 10.0f);
        }

        /*Coin Magnet*/
        struct Object* coinMag = cur_obj_nearest_object_with_behavior(bhvYellowCoin);
        struct Object* coinMagMove = cur_obj_nearest_object_with_behavior(bhvMovingYellowCoin);
        f32 oDist;
        f32 oDistMove;
        if (coinMag != NULL && m->marioObj != NULL) {
            oDist = dist_between_objects(coinMag, m->marioObj);
        }
        if (coinMagMove != NULL && m->marioObj != NULL) {
            oDistMove = dist_between_objects(coinMagMove, m->marioObj);
        }
        while (COIN_MAG == 1 && oDist != 0 && oDist >= 100 && oDist < 1000) {
            while (oDist >= 10) {
                coinMag->oPosX = approach_f32_symmetric(coinMag->oPosX, m->pos[0], 28);
                coinMag->oPosY = approach_f32_symmetric(coinMag->oPosY, m->pos[1], 28);
                coinMag->oPosZ = approach_f32_symmetric(coinMag->oPosZ, m->pos[2], 28);
                break;
            }
            break;
        }
        if (oDist == 0 && oDist > 1000) {
            obj_mark_for_deletion(coinMag);
            break;
        }
        while (COIN_MAG == 1 && coinMagMove != NULL && oDistMove >= 100 && oDistMove < 1000) {
            while (oDistMove >= 10) {
                coinMagMove->oPosX = approach_f32_symmetric(coinMagMove->oPosX, m->pos[0], 28);
                coinMagMove->oPosY = approach_f32_symmetric(coinMagMove->oPosY, m->pos[1], 28);
                coinMagMove->oPosZ = approach_f32_symmetric(coinMagMove->oPosZ, m->pos[2], 28);
                break;
            }
            break;
        }
        if (oDistMove == 0 && oDistMove > 1000) {
            obj_mark_for_deletion(coinMagMove);
            break;
        }

        /*Swim Anywhere*/
        if (SWIM_ANY == 1) {
            set_submerged_cam_preset_and_spawn_bubbles(m);
            m->waterLevel = m->pos[1] + 300;
        }

        /*No Hold Heavy*/
        if (NO_HEAVY == 1) {
            while ((m->action & ACT_GROUP_MASK) == ACT_GROUP_MOVING) {
                if (m->action == ACT_HOLD_HEAVY_WALKING) {
                    set_mario_action(m, ACT_HOLD_WALKING, 0);
                    break;
                }
                break;
            }
        }

        /*CHAOS Mode*/
        if (CHAOS_MODE == 1) {
            srand(time(NULL));
            r = rand();

            switch ((rand() % 30)) {
                case 0:
                    if (Cheats.Run <= 3) {
                        Cheats.Run += 1;
                    } else if (Cheats.Run >= 4) {
                        Cheats.Run = 0;
                    }
                    break;
                case 1:
                    if (Cheats.HugeMario == true) {
                        Cheats.HugeMario = false;
                    } else {
                        Cheats.HugeMario = true;
                    }
                    break;
                case 2:
                    if (Cheats.TinyMario == true) {
                        Cheats.TinyMario = false;
                    } else {
                        Cheats.TinyMario = true;
                    }
                    break;
                case 3:
                    if (Cheats.Moon == true) {
                        Cheats.Moon = false;
                    } else {
                        Cheats.Moon = true;
                    }
                    break;
                case 4:
                    if (Cheats.Jump == true) {
                        Cheats.Jump = false;
                    } else {
                        Cheats.Jump = true;
                    }
                    break;
                case 5:
                    play_transition(WARP_TRANSITION_FADE_FROM_COLOR, 0x3C, 0xFF, 0xFF, 0xFF);
                    break;
                case 6:
                    if (Cheats.Triple == true) {
                        Cheats.Triple = false;
                    } else {
                        Cheats.Triple = true;
                    }
                    break;
                case 7:
                    obj_spawn_yellow_coins(m->marioObj, 1);
                    break;
                case 8:
                    if (Cheats.PAC <= 6) {
                        Cheats.PAC += 1;
                    } else if (Cheats.PAC >= 7) {
                        Cheats.PAC = 0;
                    }
                    break;
                case 9:
                    hurt_and_set_mario_action(m, ACT_FORWARD_AIR_KB, 0, 0);
                    break;
                case 10:
                    cur_obj_shake_screen(SHAKE_POS_SMALL);
                    break;
                case 11:
                    m->forwardVel = (m->forwardVel + 5.0f);
                    break;
                case 12:
                    m->vel[1] -= 5;
                    break;
                case 13:
                    // Empty Slot
                    break;
                case 14:
                    // Empty Slot
                    break;
                case 15:
                    // Empty Slot
                    break;
                case 16:
                case 17:
                    while (1) {
                        print_text_fmt_int(90, 70, "CH3 %d 4T3R", m->forwardVel);
                        break;
                    }
                    break;
                case 18:
                case 19:
                    while (1) {
                        print_text_fmt_int(90, 70, "%d CHEATER %d", m->forwardVel);
                        break;
                    }
                    break;
                case 20:
                case 21:
                case 22:
                case 23:
                case 24:
                case 25:
                case 26:
                case 27:
                case 28:
                case 29:
                case 30:
                    while (1) {
                        print_text(240, 92, " ");
                        break;
                    }
                    break;
            }
        }

        /*Time Stop Cheat*/
        if (Cheats.TimeStop) {
            enable_time_stop();
        }
        if (!Cheats.TimeStop) {
            gTimeStopState &= ~(TIME_STOP_ENABLED);
        }
        /* Time Stop DynOS emhancements*/
        if (Cheats.TimeSlow && Cheats.TimeStop == false && !(m->controller->buttonDown & TIME_BUTTON)) {
            Cheats.TimeStop = true;
        }
        if (Cheats.TimeSlow == false && Cheats.TimeStop && !(m->controller->buttonDown & TIME_BUTTON)) {
            Cheats.TimeStop = false;
        }
        if (m->controller->buttonDown & TIME_BUTTON) {
            if (!Cheats.TimeSlow) {
                Cheats.TimeSlow = true;
            } else
            if (Cheats.TimeSlow) {
                Cheats.TimeSlow = false;
            }
        }


        /*FLYER Cheat*/
        if (Cheats.Fly) {
            if (m->action == ACT_FLYING) {
                if (m->controller->buttonDown & A_BUTTON) {
                    m->particleFlags |= PARTICLE_SPARKLES;
                }
            }
        }

        /*Coin Spawner*/
        switch (Cheats.Coin) {
            case 0:
                break;
            case 1:
                if (m->controller->buttonDown & B_BUTTON) {
                    obj_spawn_yellow_coins(m->marioObj, 1);
                    break;
                }
                break;
            case 2:
                if (m->controller->buttonDown & B_BUTTON) {
                    spawn_object(m->marioObj, MODEL_BLUE_COIN, bhvBlueCoinJumping);
                    break;
                }
                break;
            case 3:
                if (m->controller->buttonDown & B_BUTTON) {
                    spawn_object_relative(0, 0, 70, 150, m->marioObj, MODEL_RED_COIN, bhvRedCoin);
                    break;
                }
                break;
        }

        /*All Jumps Triple Cheat*/
        while (Cheats.Triple && (m->action & ACT_GROUP_MASK) != ACT_GROUP_SUBMERGED) {
            // While Triple Jump Cheat is true and Mario's is not underwater
            if (m->controller->buttonPressed & A_BUTTON && m->action != ACT_TRIPLE_JUMP) {
                // If A is pressed and not already triple jumping
                set_mario_action(m, ACT_TRIPLE_JUMP, 0);
                // Break out of the while
                break;
            }
            // Break out of the while
            break;
        }

        /*Hover Cheat*/
        if (Cheats.Hover) {
            set_mario_action(m, ACT_DEBUG_FREE_MOVE, 0);
            Cheats.Hover = false;
        }

        /*Moon Gravity*/
        while (Cheats.Moon) {
            while ((m->action & ACT_GROUP_MASK) == ACT_GROUP_AIRBORNE) {
                if (m->action != ACT_FREEFALL && m->action != ACT_LONG_JUMP) {
                    m->vel[1] += 2;
                    break;
                } else {
                    m->vel[1] += 1;
                    break;
                }
                break;
            }
            break;
        }
        /*Jump Modifier*/
        while (Cheats.Jump) {
            while ((m->action & ACT_GROUP_MASK) == ACT_GROUP_AIRBORNE) {
                if (m->action != ACT_FREEFALL) {
                    m->vel[1] += 1;
                    break;
                }
                if (m->action &= ACT_FREEFALL) {
                    m->vel[1] -= 5;
                    break;
                }
                break;
            }
            break;
        }

        /*Run Modifier Cheat*/
        switch (Cheats.Run) {
            case 0:
                break;
            case 1:
                if (m->action == ACT_WALKING && m->forwardVel >= 0) {
                    m->forwardVel = (m->forwardVel - 0.5f);
                }
                break;
            case 2:
                if (m->action == ACT_WALKING && m->forwardVel >= 0) {
                    m->forwardVel = (m->forwardVel - 0.7f);
                }
                break;
            case 3:
                if (m->action == ACT_WALKING && m->forwardVel >= 0) {
                    m->forwardVel = (m->forwardVel * 1.2f);
                }
                break;
            case 4:
                if (m->action == ACT_WALKING && m->forwardVel >= 0) {
                    m->forwardVel = (m->forwardVel * 1.8f);
                }
                break;
        }
        /*Play As Cheat*/
        switch(Cheats.PAC) {
            /*Model Choices*/
            case 0:
#ifdef R96
                m->marioObj->header.gfx.sharedChild = gLoadedGraphNodes[MODEL_PLAYER];
                if (isLuigi() == 1) {
                    gMarioState->animation = &Data_LuigiAnims;
                } else {
                    gMarioState->animation = &D_80339D10;
                }
                break;
#else
                m->marioObj->header.gfx.sharedChild = gLoadedGraphNodes[MODEL_MARIO]; //Use MODEL_PLAYER
                m->animation = &D_80339D10; //Only Mario's animations
                break;
#endif // R96
            case 1:
                m->marioObj->header.gfx.sharedChild = gLoadedGraphNodes[MODEL_BLACK_BOBOMB];
                m->marioObj->header.gfx.unk38.curAnim = bobomb_seg8_anims_0802396C[0];
                break;
            case 2:
                m->marioObj->header.gfx.sharedChild = gLoadedGraphNodes[MODEL_BOBOMB_BUDDY];
                m->marioObj->header.gfx.unk38.curAnim = bobomb_seg8_anims_0802396C[0];
                break;
            case 3:
                m->marioObj->header.gfx.sharedChild = gLoadedGraphNodes[MODEL_GOOMBA];
                m->marioObj->header.gfx.unk38.curAnim = goomba_seg8_anims_0801DA4C[0];
                break;
            case 4:
                m->marioObj->header.gfx.sharedChild = gLoadedGraphNodes[MODEL_KOOPA_SHELL];
                m->marioObj->header.gfx.unk38.curAnim = amp_seg8_anims_08004034[0];
                break;
            case 5:
                m->marioObj->header.gfx.sharedChild = gLoadedGraphNodes[MODEL_CHUCKYA];
                m->marioObj->header.gfx.unk38.curAnim = chuckya_seg8_anims_0800C070[0];
                break; //Forgot this in v7
            case 6:
                m->marioObj->header.gfx.sharedChild = gLoadedGraphNodes[MODEL_FLYGUY];
                m->marioObj->header.gfx.unk38.curAnim = flyguy_seg8_anims_08011A64[0];
                break;
            case 7:
                switch (gCurrLevelNum) {
                    case LEVEL_CASTLE_GROUNDS:
                        m->marioObj->header.gfx.sharedChild = gLoadedGraphNodes[MODEL_YOSHI];
                        m->marioObj->header.gfx.unk38.curAnim = yoshi_seg5_anims_05024100[0];
                        break;
                    case LEVEL_BOB:
                        m->marioObj->header.gfx.sharedChild = gLoadedGraphNodes[MODEL_KOOPA_WITH_SHELL];
                        m->marioObj->header.gfx.unk38.curAnim = koopa_seg6_anims_06011364[0];
                        m->marioObj->header.gfx.unk38.animFrame += 1;
                        break;
                    case LEVEL_BBH:
                        m->marioObj->header.gfx.sharedChild = gLoadedGraphNodes[MODEL_MAD_PIANO];
                        m->marioObj->header.gfx.unk38.curAnim = mad_piano_seg5_anims_05009B14[0];
                        break;
                    case LEVEL_WF:
                        m->marioObj->header.gfx.sharedChild = gLoadedGraphNodes[MODEL_WHOMP];
                        m->marioObj->header.gfx.unk38.curAnim = whomp_seg6_anims_06020A04[0];
                        break;
                    case LEVEL_JRB:
                        m->marioObj->header.gfx.sharedChild = gLoadedGraphNodes[MODEL_CLAM_SHELL];
                        m->marioObj->header.gfx.unk38.curAnim = clam_shell_seg5_anims_05001744[0];
                        break;
                    case LEVEL_CCM:
                        m->marioObj->header.gfx.sharedChild = gLoadedGraphNodes[MODEL_PENGUIN];
                        m->marioObj->header.gfx.unk38.curAnim = penguin_seg5_anims_05008B74[0];
                        break;
                    case LEVEL_PSS:
                        m->marioObj->header.gfx.sharedChild = gLoadedGraphNodes[MODEL_WOODEN_SIGNPOST];
                        break;
                    case LEVEL_HMC:
                        m->marioObj->header.gfx.sharedChild = gLoadedGraphNodes[MODEL_HMC_ROLLING_ROCK];
                        break;
                    case LEVEL_LLL:
                        m->marioObj->header.gfx.sharedChild = gLoadedGraphNodes[MODEL_BULLY];
                        m->marioObj->header.gfx.unk38.curAnim = bully_seg5_anims_0500470C[0];
                        break;
                    case LEVEL_SSL:
                        m->marioObj->header.gfx.sharedChild = gLoadedGraphNodes[MODEL_KLEPTO];
                        m->marioObj->header.gfx.unk38.curAnim = klepto_seg5_anims_05008CFC[0];
                        break;
                    case LEVEL_DDD:
                        m->marioObj->header.gfx.sharedChild = gLoadedGraphNodes[MODEL_SUSHI];
                        m->marioObj->header.gfx.unk38.curAnim = sushi_seg5_anims_0500AE54[0];
                        break;
                    case LEVEL_SL:
                        m->marioObj->header.gfx.sharedChild = gLoadedGraphNodes[MODEL_SPINDRIFT];
                        m->marioObj->header.gfx.unk38.curAnim = spindrift_seg5_anims_05002D68[0];
                        break;
                    case LEVEL_WDW:
                        m->marioObj->header.gfx.sharedChild = gLoadedGraphNodes[MODEL_SKEETER];
                        m->marioObj->header.gfx.unk38.curAnim = skeeter_seg6_anims_06007DE0[0];
                        break;
                    case LEVEL_TTM:
                        m->marioObj->header.gfx.sharedChild = gLoadedGraphNodes[MODEL_UKIKI];
                        m->marioObj->header.gfx.unk38.curAnim = ukiki_seg5_anims_05015784[0];
                        break;
                    case LEVEL_THI:
                        m->marioObj->header.gfx.sharedChild = gLoadedGraphNodes[MODEL_SPINY];
                        m->marioObj->header.gfx.unk38.curAnim = spiny_seg5_anims_05016EAC[0];
                        break;
                    case LEVEL_TTC:
                        m->marioObj->header.gfx.sharedChild = gLoadedGraphNodes[MODEL_THWOMP];
                        break;
                    case LEVEL_RR:
                        m->marioObj->header.gfx.sharedChild = gLoadedGraphNodes[MODEL_ENEMY_LAKITU];
                        m->marioObj->header.gfx.unk38.curAnim = lakitu_enemy_seg5_anims_050144D4[0];
                        break;
                    case LEVEL_SA:
                        m->marioObj->header.gfx.sharedChild = gLoadedGraphNodes[MODEL_MANTA_RAY];
                        m->marioObj->header.gfx.unk38.curAnim = manta_seg5_anims_05008EB4[0];
                        break;
                    case LEVEL_COTMC:
                        m->marioObj->header.gfx.sharedChild = gLoadedGraphNodes[MODEL_SNUFIT];
                        break;
                }
            }
        while (Cheats.PAC > 0) {
            /*stops softlock when dying as a Play As Character*/
            if (m->action == ACT_STANDING_DEATH) {
                level_trigger_warp(m, WARP_OP_DEATH);
                m->numLives += 1;
                update_mario_health(m);
                break;
            }
            /*Instead of making a custom hitbox for each character,
            I neutralized the only consistent problem, doors*/
            while (m->collidedObjInteractTypes & INTERACT_DOOR) {
                obj_mark_for_deletion(m->usedObj);
                spawn_object(gCurrentObject, MODEL_SMOKE, bhvBobombBullyDeathSmoke);
                obj_scale(gCurrentObject, gCurrentObject->oTimer / 4.f + 1.0f);
                gCurrentObject->oOpacity -= 14;
                gCurrentObject->oAnimState++;
                play_sound(SOUND_GENERAL2_BOBOMB_EXPLOSION, m->marioObj->header.gfx.cameraToObject);
                m->particleFlags |= PARTICLE_TRIANGLE;
                obj_set_pos(m->marioObj, 0, 0, 100);
                break;
            }
            break;
        }


        /*Speed Display*/
        if (Cheats.SPD == true) {
            print_text_fmt_int(210, 72, "SPD %d", m->forwardVel);
        }

        /*T Pose Float? Actually it's just twirling + MoonJump*/
        while (Cheats.TPF == true) {
            if (m->controller->buttonDown & A_BUTTON) {
                m->vel[1] = 25;
                set_mario_action(m, ACT_TWIRLING, 0);
            }
            break;
        }

        /*QuickEnding cheat*/
        while (Cheats.QuikEnd == true) {
            if (m->numStars == 120) {
                level_trigger_warp(m, WARP_OP_CREDITS_START);
                Cheats.QuikEnd = false;
                save_file_do_save(gCurrSaveFileNum - 1);
            }
            break;
        }

        /*AutoWallKick cheat*/
        if (Cheats.AutoWK == true && m->action == ACT_AIR_HIT_WALL) {
            m->vel[1] = 52.0f;
            m->faceAngle[1] += 0x8000;
            set_mario_action(m, ACT_WALL_KICK_AIR, 0);
            m->wallKickTimer = 0;
            set_mario_animation(m, MARIO_ANIM_START_WALLKICK);
        }

        /*HurtMario cheat*/
        while (Cheats.Hurt > 0 && m->controller->buttonDown & L_TRIG
               && m->controller->buttonPressed & A_BUTTON) {
            if (Cheats.Hurt == 1) {
                act_burning_ground(m);
            }
            if (Cheats.Hurt == 2) {
                m->flags |= MARIO_METAL_SHOCK;
                drop_and_set_mario_action(m, ACT_SHOCKED, 0);
            }
            if (Cheats.Hurt == 3) {
                hurt_and_set_mario_action(m, ACT_GROUND_BONK, 0, 1);
                play_sound(SOUND_MARIO_OOOF, m->marioObj->header.gfx.cameraToObject);
                queue_rumble_data(5, 80);
            }
            break;
        }

        /*CannonAnywhere cheat*/
        if (Cheats.Cann == true && m->controller->buttonDown & L_TRIG
            && m->controller->buttonPressed & U_CBUTTONS) {
            spawn_object_relative(1, 0, 200, 0, gCurrentObject, MODEL_NONE, bhvCannon);
        }

        /*InstantDeath cheat*/
        if (m->controller->buttonDown & L_TRIG && m->controller->buttonDown & A_BUTTON
            && m->controller->buttonPressed & B_BUTTON && m->controller->buttonDown & Z_TRIG) {
            level_trigger_warp(m, WARP_OP_DEATH);
            break;
        }

        /*CAP Cheats, this whole thing needs to be refactored, but
        I've only been adding to JAGSTAX's original patch*/
        if (Cheats.EnableCheats) {
            if (Cheats.WingCap) {
                m->flags |= MARIO_WING_CAP;
                if ((m->action & ACT_GROUP_MASK) == (!(ACT_GROUP_AIRBORNE) && !(ACT_GROUP_SUBMERGED))) {
                    set_mario_action(m, ACT_PUTTING_ON_CAP, 0);
                }
                play_cap_music(SEQ_EVENT_POWERUP);
                Cheats.WingCap = false;
            }

            if (Cheats.MetalCap) {
                m->flags |= MARIO_METAL_CAP;
                if ((m->action & ACT_GROUP_MASK) == (!(ACT_GROUP_AIRBORNE) && !(ACT_GROUP_SUBMERGED))) {
                    set_mario_action(m, ACT_PUTTING_ON_CAP, 0);
                }
                play_cap_music(SEQ_EVENT_METAL_CAP);
                Cheats.MetalCap = false;
            }

            if (Cheats.VanishCap) {
                m->flags |= MARIO_VANISH_CAP;
                if ((m->action & ACT_GROUP_MASK) == (!(ACT_GROUP_AIRBORNE) && !(ACT_GROUP_SUBMERGED))) {
                    set_mario_action(m, ACT_PUTTING_ON_CAP, 0);
                }
                play_cap_music(SEQ_EVENT_POWERUP);
                Cheats.VanishCap = false;
            }

            if (Cheats.RemoveCap) {
                m->flags &= ~MARIO_CAP_ON_HEAD;
                m->flags |= MARIO_CAP_IN_HAND;
                if ((m->action & ACT_GROUP_MASK) == (!(ACT_GROUP_AIRBORNE) && !(ACT_GROUP_SUBMERGED))) {
                    set_mario_action(m, ACT_SHIVERING, 0);
                }
                Cheats.RemoveCap = false;
            }

            if (Cheats.NormalCap) {
                m->flags &= ~MARIO_CAP_ON_HEAD;
                m->flags &= ~(MARIO_WING_CAP | MARIO_METAL_CAP | MARIO_VANISH_CAP);
                if ((m->action & ACT_GROUP_MASK) == (!(ACT_GROUP_AIRBORNE) && !(ACT_GROUP_SUBMERGED))) {
                    m->flags |= MARIO_CAP_IN_HAND;
                    set_mario_action(m, ACT_PUTTING_ON_CAP, 0);
                } else {
                    m->flags &= ~MARIO_CAP_IN_HAND;
                    m->flags |= MARIO_CAP_ON_HEAD;
                }
                stop_cap_music();
                Cheats.NormalCap = false;
            }
        }

        /* GetShell cheat */
        while (Cheats.GetShell == true && m->controller->buttonDown & L_TRIG
               && m->controller->buttonPressed & R_TRIG) {
            if (m->action & ACT_FLAG_RIDING_SHELL) {
                break;
            }

            /*This check should be added when creating a spawn cheat to prevent spamming*/
            struct Object *obj = (struct Object *) gObjectLists[OBJ_LIST_LEVEL].next;
            struct Object *first = (struct Object *) &gObjectLists[OBJ_LIST_LEVEL];
            while (obj != NULL && obj != first) {
                if (obj->header.gfx.sharedChild = gLoadedGraphNodes[MODEL_KOOPA_SHELL]) {
                    obj_mark_for_deletion(obj);
                    break;
                }
                obj = (struct Object *) obj->header.next;
            }

            if ((m->action & ACT_GROUP_MASK) == ACT_GROUP_SUBMERGED) {
                spawn_object_relative(0, 0, 100, 100, gCurrentObject, MODEL_KOOPA_SHELL,
                                      bhvKoopaShellUnderwater);
                break;
            } else {
                spawn_object_relative(0, 0, 100, 100, gCurrentObject, MODEL_KOOPA_SHELL, bhvKoopaShell);
                break;
            }
        }

        /* GetBobomb cheat */
        while (Cheats.GetBob == true && m->controller->buttonDown & L_TRIG
               && m->controller->buttonPressed & B_BUTTON) {
            spawn_object_relative(0, 0, 100, 100, gCurrentObject, MODEL_BLACK_BOBOMB, bhvBobomb);
            break;
        }

        /* SpawnCommon0 aka Spamba cheat*/
        switch (Cheats.Spamba) {
            case 0:
                break;
            case 1:
                if (m->controller->buttonDown & L_TRIG && m->controller->buttonPressed & Z_TRIG) {
                    spawn_object_relative(0, 0, 100, 100, gCurrentObject, MODEL_AMP, bhvHomingAmp);
                    break;
                }
                break;
            case 2:
                if (m->controller->buttonDown & L_TRIG && m->controller->buttonPressed & Z_TRIG) {
                    spawn_object_relative(0, 0, 0, 150, gCurrentObject, MODEL_BLUE_COIN_SWITCH,
                                          bhvBlueCoinSwitch);
                    break;
                }
                break;
            case 3:
                if (m->controller->buttonDown & L_TRIG && m->controller->buttonPressed & Z_TRIG) {
                    spawn_object_relative(0, 0, 300, 300, gCurrentObject, MODEL_BOWLING_BALL,
                                          bhvPitBowlingBall);
                    break;
                }
                break;
            case 4:
                if (m->controller->buttonDown & L_TRIG && m->controller->buttonPressed & Z_TRIG) {
                    spawn_object_relative(0, 0, 0, 200, gCurrentObject, MODEL_BREAKABLE_BOX,
                                          bhvBreakableBox);
                    break;
                }
                break;
            case 5:
                if (m->controller->buttonDown & L_TRIG && m->controller->buttonPressed & Z_TRIG) {
                    spawn_object_relative(0, 0, 50, 100, gCurrentObject, MODEL_BREAKABLE_BOX_SMALL,
                                          bhvBreakableBoxSmall);
                    break;
                }
                break;
            case 6:
                if (m->controller->buttonDown & L_TRIG && m->controller->buttonPressed & Z_TRIG) {
                    spawn_object_relative(0, 0, 10, 300, gCurrentObject, MODEL_BREAKABLE_BOX_SMALL,
                                          bhvJumpingBox);
                    break;
                }
                break;
            case 7:
                if (m->controller->buttonDown & L_TRIG && m->controller->buttonPressed & Z_TRIG) {
                    spawn_object_relative(0, 0, -10, 100, gCurrentObject, MODEL_CHECKERBOARD_PLATFORM,
                                          bhvCheckerboardPlatformSub);
                    break;
                }
                break;
            case 8:
                if (m->controller->buttonDown & L_TRIG && m->controller->buttonPressed & Z_TRIG) {
                    spawn_object_relative(0, 0, 100, 100, gCurrentObject, MODEL_CHUCKYA, bhvChuckya);
                    break;
                }
                break;
            case 9:
                if (m->controller->buttonDown & L_TRIG && m->controller->buttonPressed & Z_TRIG) {
                    spawn_object_relative(0, 0, 100, 100, gCurrentObject, MODEL_FLYGUY, bhvFlyGuy);
                    break;
                }
                break;
            case 10:
                if (m->controller->buttonDown & L_TRIG && m->controller->buttonPressed & Z_TRIG) {
                    spawn_object_relative(0, 0, 100, 100, gCurrentObject, MODEL_NONE,
                                          bhvGoombaTripletSpawner);
                    break;
                }
                break;
            case 11:
                if (m->controller->buttonDown & L_TRIG && m->controller->buttonPressed & Z_TRIG) {
                    spawn_object_relative(0, 0, 100, 100, gCurrentObject, MODEL_HEART,
                                          bhvRecoveryHeart);
                    break;
                }
                break;
            case 12:
                if (m->controller->buttonDown & L_TRIG && m->controller->buttonPressed & Z_TRIG) {
                    spawn_object_relative(0, 0, 0, 200, gCurrentObject, MODEL_METAL_BOX,
                                          bhvPushableMetalBox);
                    break;
                }
                break;
            case 13:
                if (m->controller->buttonDown & L_TRIG && m->controller->buttonPressed & Z_TRIG) {
                    spawn_object_relative(0, 0, 50, 50, gCurrentObject, MODEL_PURPLE_SWITCH,
                                          bhvPurpleSwitchHiddenBoxes);
                    break;
                }
                break;
        }

        /*Jukebox*/
        if (Cheats.JBC) {
            /*JBC is the bool, acting like the on/off*/
            switch(Cheats.JB) {
                case 0:
                    play_cap_music(SEQ_EVENT_CUTSCENE_INTRO);
                    Cheats.JBC = false;
                    break;
                case 1:
                    play_cap_music(SEQ_LEVEL_GRASS);
                    Cheats.JBC = false;
                    break;
                case 2:
                    play_cap_music(SEQ_LEVEL_INSIDE_CASTLE);
                    Cheats.JBC = false;
                    break;
                case 3:
                    play_cap_music(SEQ_LEVEL_WATER);
                    Cheats.JBC = false;
                    break;
                case 4:
                    play_cap_music(SEQ_LEVEL_HOT);
                    Cheats.JBC = false;
                    break;
                case 5:
                    play_cap_music(SEQ_LEVEL_BOSS_KOOPA);
                    Cheats.JBC = false;
                    break;
                case 6:
                    play_cap_music(SEQ_LEVEL_SNOW);
                    Cheats.JBC = false;
                    break;
                case 7:
                    play_cap_music(SEQ_LEVEL_SLIDE);
                    Cheats.JBC = false;
                    break;
                case 8:
                    play_cap_music(SEQ_LEVEL_SPOOKY);
                    Cheats.JBC = false;
                    break;
                case 9:
                    play_cap_music(SEQ_LEVEL_UNDERGROUND);
                    Cheats.JBC = false;
                    break;
                case 10:
                    play_cap_music(SEQ_LEVEL_KOOPA_ROAD);
                    Cheats.JBC = false;
                    break;
                case 11:
                    play_cap_music(SEQ_LEVEL_BOSS_KOOPA_FINAL);
                    Cheats.JBC = false;
                    break;
                case 12:
                    play_cap_music(SEQ_MENU_TITLE_SCREEN);
                    Cheats.JBC = false;
                    break;
                case 13:
                    play_cap_music(SEQ_MENU_FILE_SELECT);
                    Cheats.JBC = false;
                    break;
                case 14:
                    play_cap_music(SEQ_EVENT_POWERUP);
                    Cheats.JBC = false;
                    break;
                case 15:
                    play_cap_music(SEQ_EVENT_METAL_CAP);
                    Cheats.JBC = false;
                    break;
                case 16:
                    play_cap_music(SEQ_EVENT_BOSS);
                    Cheats.JBC = false;
                    break;
                case 17:
                    play_cap_music(SEQ_EVENT_MERRY_GO_ROUND);
                    Cheats.JBC = false;
                    break;
                case 18:
                    play_cap_music(SEQ_EVENT_CUTSCENE_CREDITS);
                    Cheats.JBC = false;
                    break;
            }
        }
        break;
    }
}