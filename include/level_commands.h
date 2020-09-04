#ifndef LEVEL_COMMANDS_H
#define LEVEL_COMMANDS_H

#include "command_macros_base.h"

#include "level_table.h"

#define OP_AND   0
#define OP_NAND  1
#define OP_EQ    2
#define OP_NEQ   3
#define OP_LT    4
#define OP_LEQ   5
#define OP_GT    6
#define OP_GEQ   7

#define OP_SET   0
#define OP_GET   1

#define VAR_CURR_SAVE_FILE_NUM  0
#define VAR_CURR_COURSE_NUM     1
#define VAR_CURR_ACT_NUM        2
#define VAR_CURR_LEVEL_NUM      3
#define VAR_CURR_AREA_INDEX     4

#define WARP_CHECKPOINT 0x80
#define WARP_NO_CHECKPOINT 0x00

#define WHIRLPOOL_COND_ALWAYS 0
#define WHIRLPOOL_COND_BOWSER2_BEATEN 2
#define WHIRLPOOL_COND_AT_LEAST_SECOND_STAR 3

// Head defines
#define REGULAR_FACE 0x0002
#define DIZZY_FACE 0x0003

#ifdef NO_SEGMENTED_MEMORY
#define EXECUTE(seg, script, scriptEnd, entry) \
    (LevelScript) CMD_BBH(0x00, 0x10, 0x0000), \
    (LevelScript) CMD_PTR(NULL), \
    (LevelScript) CMD_PTR(NULL), \
    (LevelScript) CMD_PTR(entry)

#define EXIT_AND_EXECUTE(seg, script, scriptEnd, entry) \
    (LevelScript) CMD_BBH(0x01, 0x10, 0x0000), \
    (LevelScript) CMD_PTR(NULL), \
    (LevelScript) CMD_PTR(NULL), \
    (LevelScript) CMD_PTR(entry)
#else
#define EXECUTE(seg, script, scriptEnd, entry) \
    (LevelScript) CMD_BBH(0x00, 0x10, seg), \
    (LevelScript) CMD_PTR(script), \
    (LevelScript) CMD_PTR(scriptEnd), \
    (LevelScript) CMD_PTR(entry)

#define EXIT_AND_EXECUTE(seg, script, scriptEnd, entry) \
    (LevelScript) CMD_BBH(0x01, 0x10, seg), \
    (LevelScript) CMD_PTR(script), \
    (LevelScript) CMD_PTR(scriptEnd), \
    (LevelScript) CMD_PTR(entry)
#endif

#define EXIT() \
    (LevelScript) CMD_BBH(0x02, 0x04, 0x0000)

#define SLEEP(frames) \
    (LevelScript) CMD_BBH(0x03, 0x04, frames)

#define SLEEP_BEFORE_EXIT(frames) \
    (LevelScript) CMD_BBH(0x04, 0x04, frames)

#define JUMP(target) \
    (LevelScript) CMD_BBH(0x05, 0x08, 0x0000), \
    (LevelScript) CMD_PTR(target)

#define JUMP_LINK(target) \
    (LevelScript) CMD_BBH(0x06, 0x08, 0x0000), \
    (LevelScript) CMD_PTR(target)

#define RETURN() \
    (LevelScript) CMD_BBH(0x07, 0x04, 0x0000)

#define JUMP_LINK_PUSH_ARG(arg) \
    (LevelScript) CMD_BBH(0x08, 0x04, arg)

#define JUMP_N_TIMES() \
    (LevelScript) CMD_BBH(0x09, 0x04, 0x0000)

#define LOOP_BEGIN() \
    (LevelScript) CMD_BBH(0x0A, 0x04, 0x0000)

#define LOOP_UNTIL(op, arg) \
    (LevelScript) CMD_BBBB(0x0B, 0x08, op, 0x00), \
    (LevelScript) CMD_W(arg)

#define JUMP_IF(op, arg, target) \
    (LevelScript) CMD_BBBB(0x0C, 0x0C, op, 0x00), \
    (LevelScript) CMD_W(arg), \
    (LevelScript) CMD_PTR(target)

#define JUMP_LINK_IF(op, arg, target) \
    (LevelScript) CMD_BBBB(0x0D, 0x0C, op, 0x00), \
    (LevelScript) CMD_W(arg), \
    (LevelScript) CMD_PTR(target)


#define SKIP_IF(op, arg) \
    (LevelScript) CMD_BBBB(0x0E, 0x08, op, 0) \
    (LevelScript) CMD_W(arg)

#define SKIP() \
    (LevelScript) CMD_BBH(0x0F, 0x04, 0x0000)

#define SKIP_NOP() \
    (LevelScript) CMD_BBH(0x10, 0x04, 0x0000)

#define CALL(arg, func) \
    (LevelScript) CMD_BBH(0x11, 0x08, arg), \
    (LevelScript) CMD_PTR(func)

#define CALL_LOOP(arg, func) \
    (LevelScript) CMD_BBH(0x12, 0x08, arg), \
    (LevelScript) CMD_PTR(func)

#define SET_REG(value) \
    (LevelScript) CMD_BBH(0x13, 0x04, value)

#define PUSH_POOL() \
    (LevelScript) CMD_BBH(0x14, 0x04, 0x0000)

#define POP_POOL() \
    (LevelScript) CMD_BBH(0x15, 0x04, 0x0000)

#ifdef NO_SEGMENTED_MEMORY
#define FIXED_LOAD(loadAddr, romStart, romEnd) \
    (LevelScript) CMD_BBH(0x16, 0x10, 0x0000), \
    (LevelScript) CMD_PTR(NULL), \
    (LevelScript) CMD_PTR(NULL), \
    (LevelScript) CMD_PTR(NULL)

#define LOAD_RAW(seg, romStart, romEnd) \
    (LevelScript) CMD_BBH(0x17, 0x0C, 0x0000), \
    (LevelScript) CMD_PTR(NULL), \
    (LevelScript) CMD_PTR(NULL)

#define LOAD_MIO0(seg, romStart, romEnd) \
    (LevelScript) CMD_BBH(0x18, 0x0C, 0x0000), \
    (LevelScript) CMD_PTR(NULL), \
    (LevelScript) CMD_PTR(NULL)
#else
#define FIXED_LOAD(loadAddr, romStart, romEnd) \
    (LevelScript) CMD_BBH(0x16, 0x10, 0x0000), \
    (LevelScript) CMD_PTR(loadAddr), \
    (LevelScript) CMD_PTR(romStart), \
    (LevelScript) CMD_PTR(romEnd)

#define LOAD_RAW(seg, romStart, romEnd) \
    (LevelScript) CMD_BBH(0x17, 0x0C, seg), \
    (LevelScript) CMD_PTR(romStart), \
    (LevelScript) CMD_PTR(romEnd)

#define LOAD_MIO0(seg, romStart, romEnd) \
    (LevelScript) CMD_BBH(0x18, 0x0C, seg), \
    (LevelScript) CMD_PTR(romStart), \
    (LevelScript) CMD_PTR(romEnd)
#endif

#define LOAD_MARIO_HEAD(sethead) \
    (LevelScript) CMD_BBH(0x19, 0x04, sethead)

#ifdef NO_SEGMENTED_MEMORY
#define LOAD_MIO0_TEXTURE(seg, romStart, romEnd) \
    (LevelScript) CMD_BBH(0x1A, 0x0C, 0x0000), \
    (LevelScript) CMD_PTR(NULL), \
    (LevelScript) CMD_PTR(NULL)
#else
#define LOAD_MIO0_TEXTURE(seg, romStart, romEnd) \
    (LevelScript) CMD_BBH(0x1A, 0x0C, seg), \
    (LevelScript) CMD_PTR(romStart), \
    (LevelScript) CMD_PTR(romEnd)
#endif

#define INIT_LEVEL() \
    (LevelScript) CMD_BBH(0x1B, 0x04, 0x0000)

#define CLEAR_LEVEL() \
    (LevelScript) CMD_BBH(0x1C, 0x04, 0x0000)

#define ALLOC_LEVEL_POOL() \
    (LevelScript) CMD_BBH(0x1D, 0x04, 0x0000)

#define FREE_LEVEL_POOL() \
    (LevelScript) CMD_BBH(0x1E, 0x04, 0x0000)

#define AREA(index, geo) \
    (LevelScript) CMD_BBBB(0x1F, 0x08, index, 0), \
    (LevelScript) CMD_PTR(geo)

#define END_AREA() \
    (LevelScript) CMD_BBH(0x20, 0x04, 0x0000)

#define LOAD_MODEL_FROM_DL(model, dl, layer) \
    (LevelScript) CMD_BBH(0x21, 0x08, ((layer << 12) | model)), \
    (LevelScript) CMD_PTR(dl)

#define LOAD_MODEL_FROM_GEO(model, geo) \
    (LevelScript) CMD_BBH(0x22, 0x08, model), \
    (LevelScript) CMD_PTR(geo)

// unk8 is float, but doesn't really matter since CMD23 is unused
#define CMD23(model, unk4, unk8) \
    (LevelScript) CMD_BBH(0x22, 0x08, model), \
    (LevelScript) CMD_PTR(unk4), \
    (LevelScript) CMD_W(unk8)

#define OBJECT_WITH_ACTS(model, posX, posY, posZ, angleX, angleY, angleZ, behParam, beh, acts) \
    (LevelScript) CMD_BBBB(0x24, 0x18, acts, model), \
    (LevelScript) CMD_HHHHHH(posX, posY, posZ, angleX, angleY, angleZ), \
    (LevelScript) CMD_W(behParam), \
    (LevelScript) CMD_PTR(beh)

#define OBJECT(model, posX, posY, posZ, angleX, angleY, angleZ, behParam, beh) \
    (LevelScript) OBJECT_WITH_ACTS(model, posX, posY, posZ, angleX, angleY, angleZ, behParam, beh, 0x1F)

#define MARIO(unk3, behArg, beh) \
    (LevelScript) CMD_BBBB(0x25, 0x0C, 0x00, unk3), \
    (LevelScript) CMD_W(behArg), \
    (LevelScript) CMD_PTR(beh)

#define WARP_NODE(id, destLevel, destArea, destNode, flags) \
    (LevelScript) CMD_BBBB(0x26, 0x08, id, destLevel), \
    (LevelScript) CMD_BBBB(destArea, destNode, flags, 0x00)

#define PAINTING_WARP_NODE(id, destLevel, destArea, destNode, flags) \
    (LevelScript) CMD_BBBB(0x27, 0x08, id, destLevel), \
    (LevelScript) CMD_BBBB(destArea, destNode, flags, 0x00)

#define INSTANT_WARP(index, destArea, displaceX, displaceY, displaceZ) \
    (LevelScript) CMD_BBBB(0x28, 0x0C, index, destArea), \
    (LevelScript) CMD_HH(displaceX, displaceY), \
    (LevelScript) CMD_HH(displaceZ, 0x0000)

#define LOAD_AREA(area) \
    (LevelScript) CMD_BBBB(0x29, 0x04, area, 0x00)

#define CMD2A(unk2) \
    (LevelScript) CMD_BBBB(0x2A, 0x04, unk2, 0x00)

#define MARIO_POS(area, yaw, posX, posY, posZ) \
    (LevelScript) CMD_BBBB(0x2B, 0x0C, area, 0x00), \
    (LevelScript) CMD_HH(yaw, posX), \
    (LevelScript) CMD_HH(posY, posZ)

// unused
#define CMD2C() \
    (LevelScript) CMD_BBH(0x2C, 0x04, 0x0000)

// unused
#define CMD2D() \
    (LevelScript) CMD_BBH(0x2D, 0x04, 0x0000)

#define TERRAIN(terrainData) \
    (LevelScript) CMD_BBH(0x2E, 0x08, 0x0000), \
    (LevelScript) CMD_PTR(terrainData)

#define ROOMS(surfaceRooms) \
    (LevelScript) CMD_BBH(0x2F, 0x08, 0x0000), \
    (LevelScript) CMD_PTR(surfaceRooms)

#define SHOW_DIALOG(index, dialogId) \
    (LevelScript) CMD_BBBB(0x30, 0x04, index, dialogId)

#define TERRAIN_TYPE(terrainType) \
    (LevelScript) CMD_BBH(0x31, 0x04, terrainType)

#define NOP() \
    (LevelScript) CMD_BBH(0x32, 0x04, 0x0000)

#define TRANSITION(transType, time, colorR, colorG, colorB) \
    (LevelScript) CMD_BBBB(0x33, 0x08, transType, time), \
    (LevelScript) CMD_BBBB(colorR, colorG, colorB, 0x00)

#define BLACKOUT(active) \
    (LevelScript) CMD_BBBB(0x34, 0x04, active, 0x00)

#define GAMMA(enabled) \
    (LevelScript) CMD_BBBB(0x35, 0x04, enabled, 0x00)

#define SET_BACKGROUND_MUSIC(settingsPreset, seq) \
    (LevelScript) CMD_BBH(0x36, 0x08, settingsPreset), \
    (LevelScript) CMD_HH(seq, 0x0000)

#define SET_MENU_MUSIC(seq) \
    (LevelScript) CMD_BBH(0x37, 0x04, seq)

#define STOP_MUSIC(fadeOutTime) \
    (LevelScript) CMD_BBH(0x38, 0x04, fadeOutTime)

#define MACRO_OBJECTS(objList) \
    (LevelScript) CMD_BBH(0x39, 0x08, 0x0000), \
    (LevelScript) CMD_PTR(objList)

// unused
#define CMD3A(unk2, unk4, unk6, unk8, unk10) \
    (LevelScript) CMD_BBH(0x3A, 0x0C, unk2), \
    (LevelScript) CMD_HH(unk6, unk8), \
    (LevelScript) CMD_HH(unk10, 0x0000)

#define WHIRLPOOL(index, condition, posX, posY, posZ, strength) \
    (LevelScript) CMD_BBBB(0x3B, 0x0C, index, condition), \
    (LevelScript) CMD_HH(posX, posY), \
    (LevelScript) CMD_HH(posZ, strength)

#define GET_OR_SET(op, var) \
    (LevelScript) CMD_BBBB(0x3C, 0x04, op, var)

#endif // LEVEL_COMMANDS_H
