#include "dynos.cpp.h"
extern "C" {
#include "sm64.h"
#include "seq_ids.h"
#include "course_table.h"
#include "audio/external.h"
#include "engine/surface_collision.h"
#include "game/mario.h"
#include "game/ingame_menu.h"
#include "game/level_update.h"
#include "game/sound_init.h"
#include "game/object_list_processor.h"
#include "game/options_menu.h"
extern s8 gDialogBoxState;
extern s16 gMenuMode;
extern s32 gWdwWaterLevelSet;
extern const u8 sSpawnTypeFromWarpBhv[];
extern void set_mario_initial_action(struct MarioState *, u32, u32);
extern void set_play_mode(s16);
}

//
// Data
//

#ifndef DYNOS_COOP
       s32 gDDDBowsersSub     = -1;
       s32 gDDDPoles          = -1;
#endif
static s32 sDynosWarpLevelNum = -1;
static s32 sDynosWarpAreaNum  = -1;
static s32 sDynosWarpActNum   = -1;
static s32 sDynosExitLevelNum = -1;
static s32 sDynosExitAreaNum  = -1;

//
// Level Entry
//

bool DynOS_Warp_ToLevel(s32 aLevel, s32 aArea, s32 aAct) {
    if (DynOS_Level_GetCourse(aLevel) == COURSE_NONE || !DynOS_Level_GetWarpEntry(aLevel, aArea)) {
        return false;
    }

    sDynosWarpLevelNum = aLevel;
    sDynosWarpAreaNum  = aArea;
    sDynosWarpActNum   = aAct;
    return true;
}

bool DynOS_Warp_RestartLevel() {
    return DynOS_Warp_ToLevel(gCurrLevelNum, 1, gCurrActNum);
}

//
// Level Exit
//

bool DynOS_Warp_ExitLevel(s32 aDelay) {
    if (DynOS_Level_GetCourse(gCurrLevelNum) == COURSE_NONE) {
        return false;
    }

    // Close the pause menu if it was open
    optmenu_toggle();
    level_set_transition(0, NULL);
    gDialogBoxState = 0;
    gMenuMode = -1;

    // Cancel out every music/sound/sequence
    for (u16 seqid = 0; seqid != SEQ_COUNT; ++seqid) {
    stop_background_music(seqid);
    }
    play_shell_music();
    stop_shell_music();
    stop_cap_music();
    func_80321080(0);
    fadeout_music(0);
    fadeout_level_music(0);

    // Play Mario head transition, and change play mode to avoid getting stuck on the pause menu
    aDelay = MAX(1, aDelay);
    gMarioState->invincTimer = -1;
    play_transition(WARP_TRANSITION_FADE_INTO_MARIO, aDelay, 0x00, 0x00, 0x00);
    set_play_mode(0);
    sDynosExitLevelNum = gCurrLevelNum;
    sDynosExitAreaNum = gCurrAreaIndex;
    return true;
}

bool DynOS_Warp_ToCastle(s32 aLevel) {
    if (DynOS_Level_GetCourse(aLevel) == COURSE_NONE) {
        return false;
    }

    // Close the pause menu if it was open
    optmenu_toggle();
    level_set_transition(0, NULL);
    gDialogBoxState = 0;
    gMenuMode = -1;

    // Cancel out every music/sound/sequence
    for (u16 seqid = 0; seqid != SEQ_COUNT; ++seqid) {
    stop_background_music(seqid);
    }
    play_shell_music();
    stop_shell_music();
    stop_cap_music();
    func_80321080(0);
    fadeout_music(0);
    fadeout_level_music(0);

    // Change play mode to avoid getting stuck on the pause menu
    set_play_mode(0);
    sDynosExitLevelNum = aLevel;
    sDynosExitAreaNum = 1;
    return true;
}

//
// Params
//

#ifndef DYNOS_COOP
const char *DynOS_Warp_GetParamName(s32 aLevel, s32 aIndex) {
    static const char *sLevelParams[][5] = {
        { "", "", "", "", "" },
        { "None", "No Submarine, No Poles", "Submarine Only", "Poles Only", "Submarine And Poles" },
        { "None", "Water Level: Lowest", "Water Level: Low", "Water Level: High", "Water Level: Highest" },
        { "None", "Top Flooded", "Top Drained", "Top Flooded", "Top Drained" },
        { "None", "Clock Speed: Stopped", "Clock Speed: Slow", "Clock Speed: Fast", "Clock Speed: Random" },
    };
    switch (aLevel) {
        case LEVEL_DDD: return sLevelParams[1][MIN(4, aIndex)];
        case LEVEL_WDW: return sLevelParams[2][MIN(4, aIndex)];
        case LEVEL_THI: return sLevelParams[3][MIN(4, aIndex)];
        case LEVEL_TTC: return sLevelParams[4][MIN(4, aIndex)];
    }
    return sLevelParams[0][MIN(4, aIndex)];
}

// Called thrice
// Pass -1 to use the previous value (only once)
void DynOS_Warp_SetParam(s32 aLevel, s32 aIndex) {
    static s32 sDynosWarpPrevParamIndex = -1;
    if (aIndex == -1) {
        aIndex = sDynosWarpPrevParamIndex;
        sDynosWarpPrevParamIndex = -1;
    } else {
        sDynosWarpPrevParamIndex = aIndex;
    }

    switch (aLevel) {
    case LEVEL_DDD:
        switch (aIndex) {
            case 1: gDDDBowsersSub = 0; gDDDPoles = 0; break;
            case 2: gDDDBowsersSub = 1; gDDDPoles = 0; break;
            case 3: gDDDBowsersSub = 0; gDDDPoles = 1; break;
            case 4: gDDDBowsersSub = 1; gDDDPoles = 1; break;
        }
        break;

    case LEVEL_WDW:
        if (gEnvironmentRegions) {
        switch (aIndex) {
            case 1: gEnvironmentRegions[6] = *gEnvironmentLevels =   31; gWdwWaterLevelSet = 1; break;
            case 2: gEnvironmentRegions[6] = *gEnvironmentLevels = 1024; gWdwWaterLevelSet = 1; break;
            case 3: gEnvironmentRegions[6] = *gEnvironmentLevels = 1792; gWdwWaterLevelSet = 1; break;
            case 4: gEnvironmentRegions[6] = *gEnvironmentLevels = 2816; gWdwWaterLevelSet = 1; break;
        }
        }
        break;

    case LEVEL_THI:
        switch (aIndex) {
            case 1: gTHIWaterDrained = 0; break;
            case 2: gTHIWaterDrained = 1; break;
            case 3: gTHIWaterDrained = 0; break;
            case 4: gTHIWaterDrained = 1; break;
        }
        break;

    case LEVEL_TTC:
        switch (aIndex) {
            case 1: gTTCSpeedSetting = TTC_SPEED_STOPPED; break;
            case 2: gTTCSpeedSetting = TTC_SPEED_SLOW; break;
            case 3: gTTCSpeedSetting = TTC_SPEED_FAST; break;
            case 4: gTTCSpeedSetting = TTC_SPEED_RANDOM; break;
        }
        break;
    }
}
#endif

//
// Update
//

static void *DynOS_Warp_UpdateWarp(void *aCmd, bool aIsLevelInitDone) {
    static s32 sDynosWarpTargetArea = -1;

    // Phase 1 - Clear the previous level and set up the new level
    if (sDynosWarpTargetArea == -1) {

        // Close the pause menu if it was open
        optmenu_toggle();
        level_set_transition(0, NULL);
        gDialogBoxState = 0;
        gMenuMode = -1;

        // Cancel out every music/sound/sequence
        for (u16 seqid = 0; seqid != SEQ_COUNT; ++seqid) {
        stop_background_music(seqid);
        }
        play_shell_music();
        stop_shell_music();
        stop_cap_music();
        func_80321080(0);
        fadeout_music(0);
        fadeout_level_music(0);

        // Free everything from the current level
        clear_objects();
        clear_area_graph_nodes();
        clear_areas();
        main_pool_pop_state();

        // Reset Mario's state
        gMarioState->healCounter = 0;
        gMarioState->hurtCounter = 0;
        gMarioState->numCoins = 0;
        gMarioState->input = 0;
        gMarioState->controller->buttonPressed = 0;
        gHudDisplay.coins = 0;

        // Set up new level values
        gCurrLevelNum = sDynosWarpLevelNum;
        gCurrCourseNum = DynOS_Level_GetCourse(gCurrLevelNum);
        gSavedCourseNum = gCurrCourseNum;
        gCurrActNum = MAX(1, sDynosWarpActNum * (gCurrCourseNum <= COURSE_STAGES_MAX));
        gDialogCourseActNum = gCurrActNum;
        gCurrAreaIndex = sDynosWarpAreaNum;
#ifndef DYNOS_COOP
        DynOS_Warp_SetParam(gCurrLevelNum, DynOS_Opt_GetValue("dynos_warp_param"));
#endif
        sDynosWarpTargetArea = gCurrAreaIndex;

        // Set up new level script
        sWarpDest.type = 0;
        sWarpDest.levelNum = 0;
        sWarpDest.areaIdx = gCurrAreaIndex;
        sWarpDest.nodeId = 0;
        sWarpDest.arg = 0;
        return (void *) DynOS_Level_GetScript(gCurrLevelNum);

    } else {
    
        // Phase 2 - Set Mario spawn info after the MARIO_POS command
        if (*((u8 *) aCmd) == 0x2B) {
            gMarioSpawnInfo->areaIndex = sDynosWarpTargetArea;
            gCurrAreaIndex = sDynosWarpTargetArea;
        }

        // Phase 3 - End level initialization
        if (aIsLevelInitDone) {

            // Init Mario
            s16 *_LevelEntryWarp = DynOS_Level_GetWarpEntry(gCurrLevelNum, gCurrAreaIndex);
            s16 sDynosWarpSpawnType = sSpawnTypeFromWarpBhv[_LevelEntryWarp[2]];
            gMarioSpawnInfo->startPos[0] = _LevelEntryWarp[3] + (sDynosWarpSpawnType == MARIO_SPAWN_DOOR_WARP) * 300.0f * sins(_LevelEntryWarp[6]);
            gMarioSpawnInfo->startPos[1] = _LevelEntryWarp[4];
            gMarioSpawnInfo->startPos[2] = _LevelEntryWarp[5] + (sDynosWarpSpawnType == MARIO_SPAWN_DOOR_WARP) * 300.0f * coss(_LevelEntryWarp[6]);
            gMarioSpawnInfo->startAngle[0] = 0;
            gMarioSpawnInfo->startAngle[1] = _LevelEntryWarp[6];
            gMarioSpawnInfo->startAngle[2] = 0;
            gMarioSpawnInfo->areaIndex = gCurrAreaIndex;
            init_mario();
            set_mario_initial_action(gMarioState, sDynosWarpSpawnType, 0);
#ifndef DYNOS_COOP
            DynOS_Warp_SetParam(gCurrLevelNum, DynOS_Opt_GetValue("dynos_warp_param"));
#endif

            // Init transition
            reset_camera(gCurrentArea->camera);
            init_camera(gCurrentArea->camera);
            sDelayedWarpOp = WARP_OP_NONE;
            switch (sDynosWarpSpawnType) {
                case MARIO_SPAWN_UNKNOWN_03:           play_transition(WARP_TRANSITION_FADE_FROM_STAR,   0x10, 0x00, 0x00, 0x00); break;
                case MARIO_SPAWN_DOOR_WARP:            play_transition(WARP_TRANSITION_FADE_FROM_CIRCLE, 0x10, 0x00, 0x00, 0x00); break;
                case MARIO_SPAWN_TELEPORT:             play_transition(WARP_TRANSITION_FADE_FROM_COLOR,  0x14, 0xFF, 0xFF, 0xFF); break;
                case MARIO_SPAWN_SPIN_AIRBORNE:        play_transition(WARP_TRANSITION_FADE_FROM_COLOR,  0x1A, 0xFF, 0xFF, 0xFF); break;
                case MARIO_SPAWN_SPIN_AIRBORNE_CIRCLE: play_transition(WARP_TRANSITION_FADE_FROM_CIRCLE, 0x10, 0x00, 0x00, 0x00); break;
                case MARIO_SPAWN_UNKNOWN_27:           play_transition(WARP_TRANSITION_FADE_FROM_COLOR,  0x10, 0x00, 0x00, 0x00); break;
                default:                               play_transition(WARP_TRANSITION_FADE_FROM_STAR,   0x10, 0x00, 0x00, 0x00); break;
            }

            // Set music
            set_background_music(gCurrentArea->musicParam, gCurrentArea->musicParam2, 0);
            if (gMarioState->flags & MARIO_METAL_CAP)  play_cap_music(SEQUENCE_ARGS(4, SEQ_EVENT_METAL_CAP));
            if (gMarioState->flags & MARIO_VANISH_CAP) play_cap_music(SEQUENCE_ARGS(4, SEQ_EVENT_POWERUP));
            if (gMarioState->flags & MARIO_WING_CAP)   play_cap_music(SEQUENCE_ARGS(4, SEQ_EVENT_POWERUP));
            if (gCurrLevelNum == LEVEL_BOWSER_1 ||
                gCurrLevelNum == LEVEL_BOWSER_2 ||
                gCurrLevelNum == LEVEL_BOWSER_3) {
                sound_banks_enable(0, 0xFFFF); // Bowser levels sound fix
            }

            // Reset values
            sDynosWarpTargetArea = -1;
            sDynosWarpLevelNum   = -1;
            sDynosWarpAreaNum    = -1;
            sDynosWarpActNum     = -1;
        }
    }

    return NULL;
}

static void DynOS_Warp_FindExitPosition(s16 &aPosX, s16 &aPosY, s16 &aPosZ, s16 aFYaw, f32 aDist) {
    for (f32 _Dist = aDist; _Dist > 0.f; _Dist -= 10.f) {
        f32 _PosX = (f32) aPosX + _Dist * sins(aFYaw + 0x8000);
        f32 _PosZ = (f32) aPosZ + _Dist * coss(aFYaw + 0x8000);
        for (f32 _DeltaY = 0.f; _DeltaY <= 5000.f; _DeltaY += 100.f) {
            f32 _PosY = (f32) aPosY + _DeltaY;
            struct Surface *_Floor;
            f32 _FloorY = find_floor(_PosX, _PosY, _PosZ, &_Floor);
            if (_Floor &&
                _Floor->type != SURFACE_WARP &&
                _Floor->type != SURFACE_BURNING &&
                _Floor->type != SURFACE_DEATH_PLANE &&
                _Floor->type != SURFACE_VERTICAL_WIND &&
                _Floor->type != SURFACE_DEEP_QUICKSAND &&
                _Floor->type != SURFACE_INSTANT_QUICKSAND &&
                _Floor->type != SURFACE_INSTANT_MOVING_QUICKSAND) {
                aPosX = _PosX;
                aPosY = _FloorY;
                aPosZ = _PosZ;
                return;
            }
        }
    }
}

static void *DynOS_Warp_UpdateExit(void *aCmd, bool aIsLevelInitDone) {
    static s32  sDynosExitTargetArea = -1;
    static s16 *sDynosExitTargetWarp = NULL;

    // Phase 0 - Wait for the Mario head transition to end
    if (sDynosExitTargetArea == -1 && DynOS_IsTransitionActive()) {
        return NULL;
    }

    // Phase 1 - Clear the previous level and set up the new level
    if (sDynosExitTargetArea == -1) {

        // Bowser levels
        if (sDynosExitLevelNum == LEVEL_BOWSER_1) sDynosExitLevelNum = LEVEL_BITDW;
        if (sDynosExitLevelNum == LEVEL_BOWSER_2) sDynosExitLevelNum = LEVEL_BITFS;
        if (sDynosExitLevelNum == LEVEL_BOWSER_3) sDynosExitLevelNum = LEVEL_BITS;

        // Exit warp to Castle warp
        // Uses the death warp, as it's the only warp that exists for every stage in the game
        s16 *_ExitWarp = DynOS_Level_GetWarpDeath(sDynosExitLevelNum, sDynosExitAreaNum);
        sDynosExitTargetWarp = DynOS_Level_GetWarp(_ExitWarp[7], _ExitWarp[8], _ExitWarp[9]);

        // Free everything from the current level
        clear_objects();
        clear_area_graph_nodes();
        clear_areas();
        main_pool_pop_state();

        // Reset Mario's state
        gMarioState->healCounter = 0;
        gMarioState->hurtCounter = 0;
        gMarioState->numCoins = 0;
        gMarioState->input = 0;
        gMarioState->controller->buttonPressed = 0;
        gHudDisplay.coins = 0;

        // Set up new level values
        gCurrLevelNum = _ExitWarp[7];
        gCurrCourseNum = DynOS_Level_GetCourse(gCurrLevelNum);
        gSavedCourseNum = gCurrCourseNum;
        gDialogCourseActNum = gCurrActNum;
        gCurrAreaIndex = _ExitWarp[8];
        sDynosExitTargetArea = _ExitWarp[8];

        // Set up new level script
        sWarpDest.type = 0;
        sWarpDest.levelNum = 0;
        sWarpDest.areaIdx = gCurrAreaIndex;
        sWarpDest.nodeId = 0;
        sWarpDest.arg = 0;
        return (void *) DynOS_Level_GetScript(gCurrLevelNum);

    } else {
    
        // Phase 2 - Set Mario spawn info after the MARIO_POS command
        if (*((u8 *) aCmd) == 0x2B) {
            gMarioSpawnInfo->areaIndex = sDynosExitTargetArea;
            gCurrAreaIndex = sDynosExitTargetArea;
        }

        // Phase 3 - End level initialization
        if (sDynosExitTargetWarp && aIsLevelInitDone) {
            
            // Find target position
            // Because of course, every hack has its own warp distances and orientations...
            s16 _TargetPosX = sDynosExitTargetWarp[3];
            s16 _TargetPosY = sDynosExitTargetWarp[4];
            s16 _TargetPosZ = sDynosExitTargetWarp[5];
            s16 _TargetFYaw = sDynosExitTargetWarp[6];
            s16 _TargetDYaw = 0;
            f32 _TargetDist = 500.f;
            DynOS_Warp_FindExitPosition(_TargetPosX, _TargetPosY, _TargetPosZ, _TargetFYaw + _TargetDYaw, _TargetDist);

            // Init Mario
            gMarioSpawnInfo->startPos[0] = _TargetPosX;
            gMarioSpawnInfo->startPos[1] = _TargetPosY;
            gMarioSpawnInfo->startPos[2] = _TargetPosZ;
            gMarioSpawnInfo->startAngle[0] = 0;
            gMarioSpawnInfo->startAngle[1] = _TargetFYaw + _TargetDYaw;
            gMarioSpawnInfo->startAngle[2] = 0;
            gMarioSpawnInfo->areaIndex = gCurrAreaIndex;
            init_mario();
            set_mario_initial_action(gMarioState, MARIO_SPAWN_UNKNOWN_02, 0);

            // Init transition
            reset_camera(gCurrentArea->camera);
            init_camera(gCurrentArea->camera);
            sDelayedWarpOp = WARP_OP_NONE;
            play_transition(WARP_TRANSITION_FADE_FROM_STAR, 15, 0x00, 0x00, 0x00);
            play_sound(SOUND_MENU_MARIO_CASTLE_WARP, gDefaultSoundArgs);

            // Set music
            set_background_music(gCurrentArea->musicParam, gCurrentArea->musicParam2, 0);
            sDynosExitTargetWarp = NULL;
        }

        // Phase 4 - Unlock Mario as soon as the second transition is ended
        if (!sDynosExitTargetWarp && !DynOS_IsTransitionActive()) {
            sDynosExitTargetArea = -1;
            sDynosExitLevelNum   = -1;
            sDynosExitAreaNum    = -1;
        }
    }

    return NULL;
}

void *DynOS_Warp_Update(void *aCmd, bool aIsLevelInitDone) {

    // Level Exit
    if (sDynosExitLevelNum != -1 &&
        sDynosExitAreaNum != -1) {
        return DynOS_Warp_UpdateExit(aCmd, aIsLevelInitDone);
    }

    // Level Warp
    if (sDynosWarpLevelNum != -1 &&
        sDynosWarpAreaNum != -1 &&
        sDynosWarpActNum != -1) {
        return DynOS_Warp_UpdateWarp(aCmd, aIsLevelInitDone);
    }

#ifndef DYNOS_COOP
    // Reset DDD settings to default
    if (gCurrCourseNum == COURSE_NONE) {
        gDDDBowsersSub = -1;
        gDDDPoles = -1;
    }
#endif

    return NULL;
}
