#include "dynos.cpp.h"
extern "C" {
#include "sm64.h"
#include "level_commands.h"
#include "game/level_update.h"
#include "game/options_menu.h"
#include "game/object_list_processor.h"
extern s16 gMenuMode;
extern s8 gDialogBoxState;
#ifdef OMM_DEFINES_H
extern void omm_opt_init();
#endif
}

#ifdef DYNOS_COOP
//
// Network (Coop only)
//

extern "C" {
#include "pc/network/network.h"
}

static struct DynosCoopCommandStruct {
    s32 mType = DYNOS_COOP_COMMAND_NONE;
    s32 mLevel = 0;
    s32 mArea = 0;
    s32 mAct = 0;
} sDynosCoopCommand;
static u8 sDynosCoopCustomPacketId = 0xFF;

static void DynOS_Coop_SendPacket(struct Packet *p, UNUSED void *params) {
    packet_write(p, &sDynosCoopCommand, sizeof(DynosCoopCommandStruct));
}

static void DynOS_Coop_ReceivePacket(struct Packet *p) {
    packet_read(p, &sDynosCoopCommand, sizeof(DynosCoopCommandStruct));
}

static void DynOS_Coop_Update() {
    if (sDynosCoopCustomPacketId == 0xFF) {
        network_register_mod((char *) "DynOS.1.0.coop");
        sDynosCoopCustomPacketId = network_register_custom_packet(DynOS_Coop_SendPacket, DynOS_Coop_ReceivePacket);
    }

    // Commands
    switch (sDynosCoopCommand.mType) {
        case DYNOS_COOP_COMMAND_WARP_TO_LEVEL: {
            DynOS_Warp_ToLevel(sDynosCoopCommand.mLevel, sDynosCoopCommand.mArea, sDynosCoopCommand.mAct);
        } break;

        case DYNOS_COOP_COMMAND_WARP_TO_CASTLE: {
            DynOS_Warp_ToCastle(sDynosCoopCommand.mLevel);
        } break;

        case DYNOS_COOP_COMMAND_RESTART_LEVEL: {
            DynOS_Warp_RestartLevel();
        } break;

        case DYNOS_COOP_COMMAND_EXIT_LEVEL: {
            DynOS_Warp_ExitLevel(30);
        } break;
    }
    sDynosCoopCommand.mType = DYNOS_COOP_COMMAND_NONE;
}

void DynOS_Coop_SendCommand(s32 aType, s32 aLevel, s32 aArea, s32 aAct) {
    sDynosCoopCommand.mType = aType;
    sDynosCoopCommand.mLevel = aLevel;
    sDynosCoopCommand.mArea = aArea;
    sDynosCoopCommand.mAct = aAct;
    network_send_custom(sDynosCoopCustomPacketId, true, false, NULL);
    sDynosCoopCommand.mType = DYNOS_COOP_COMMAND_NONE;
}
#endif

//
// Main Menu
//

#ifndef DYNOS_COOP
void DynOS_ReturnToMainMenu() {
    optmenu_toggle();
    level_set_transition(0, NULL);
    gDialogBoxState = 0;
    gMenuMode = -1;
    fade_into_special_warp(-2, 0);
}
#endif

//
// Init
//

DYNOS_AT_STARTUP void DynOS_Init() {
#ifdef OMM_DEFINES_H
    omm_opt_init();
#endif
    DynOS_Opt_Init();
}

//
// Update
//

static bool sDynosIsLevelEntry = false;
void DynOS_UpdateOpt(void *aPad) {
    if (sDynosIsLevelEntry) {
#ifndef DYNOS_COOP
        DynOS_Warp_SetParam(gCurrLevelNum, -1);
#endif
        sDynosIsLevelEntry = false;
    }
    DynOS_Opt_Update((OSContPad *) aPad);
#ifdef DYNOS_COOP
    DynOS_Coop_Update();
#endif
    gPrevFrameObjectCount = 0;
}

void *DynOS_UpdateCmd(void *aCmd) {
    static const uintptr_t sCmdLevelEntry[] = { CALL(0, lvl_init_or_update) };
    sDynosIsLevelEntry |= (memcmp(aCmd, sCmdLevelEntry, sizeof(sCmdLevelEntry)) == 0);
    return DynOS_Warp_Update(aCmd, sDynosIsLevelEntry);
}

void DynOS_UpdateGfx() {
    DynOS_Gfx_Update();
}

bool DynOS_IsTransitionActive() {
    return gWarpTransition.isActive;
}
