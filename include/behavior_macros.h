#pragma once

// TODO: Clean up these imports.

#define OBJECT_FIELDS_INDEX_DIRECTLY

#include "sm64.h"

#include "object_constants.h"
#include "game/object_list_processor.h"
#include "game/interaction.h"
#include "game/behavior_actions.h"
#include "game/behaviors/thwomp/thwomp.hpp"
#include "game/mario_actions_cutscene.h"
#include "game/mario_misc.h"
#include "game/object_helpers.h"
#include "game/debug.h"
#include "menu/file_select.hpp"
#include "engine/surface_load.h"

#include "actors/common0.h"
#include "actors/common1.h"
#include "actors/group1.h"
#include "actors/group2.h"
#include "actors/group3.h"
#include "actors/group4.h"
#include "actors/group5.h"
#include "actors/group6.h"
#include "actors/group7.h"
#include "actors/group8.h"
#include "actors/group9.h"
#include "actors/group10.h"
#include "actors/group11.h"
#include "actors/group12.h"
#include "actors/group13.h"
#include "actors/group14.h"
#include "actors/group15.h"
#include "actors/group16.h"
#include "actors/group17.h"
#include "levels/bbh/header.h"
#include "levels/castle_inside/header.h"
#include "levels/hmc/header.h"
#include "levels/ssl/header.h"
#include "levels/bob/header.h"
#include "levels/sl/header.h"
#include "levels/wdw/header.h"
#include "levels/jrb/header.h"
#include "levels/thi/header.h"
#include "levels/ttc/header.h"
#include "levels/rr/header.h"
#include "levels/castle_grounds/header.h"
#include "levels/bitdw/header.h"
#include "levels/lll/header.h"
#include "levels/sa/header.h"
#include "levels/bitfs/header.h"
#include "levels/ddd/header.h"
#include "levels/wf/header.h"
#include "levels/bowser_2/header.h"
#include "levels/ttm/header.h"

#include "make_const_nonconst.h"
#include "behavior_data.h"
#include "include/behavior_macros.h"

#define BC_B(a) _SHIFTL(a, 24, 8)
#define BC_BB(a, b) (_SHIFTL(a, 24, 8) | _SHIFTL(b, 16, 8))
#define BC_BBBB(a, b, c, d) \
  (_SHIFTL(a, 24, 8) | _SHIFTL(b, 16, 8) | _SHIFTL(c, 8, 8) | _SHIFTL(d, 0, 8))
#define BC_BBH(a, b, c) \
  (_SHIFTL(a, 24, 8) | _SHIFTL(b, 16, 8) | _SHIFTL(c, 0, 16))
#define BC_B0H(a, b) (_SHIFTL(a, 24, 8) | _SHIFTL(b, 0, 16))
#define BC_H(a) _SHIFTL(a, 16, 16)
#define BC_HH(a, b) (_SHIFTL(a, 16, 16) | _SHIFTL(b, 0, 16))
#define BC_W(a) ((uintptr_t)(u32)(a))
#define BC_PTR(a) ((uintptr_t)(a))

// Defines the start of the behavior script as well as the object list the
// object belongs to. Has some special behavior for certain objects.
#define BEGIN(objList) BC_BB(0x00, objList)

// Delays the behavior script for a certain number of frames.
#define DELAY(num) BC_B0H(0x01, num)

// Jumps to a new behavior command and stores the return address in the object's
// stack.
#define CALL(addr) BC_B(0x02), BC_PTR(addr)

// Jumps back to the behavior command stored in the object's stack.
#define RETURN() BC_B(0x03)

// Jumps to a new behavior script without saving anything.
#define GOTO(addr) BC_B(0x04), BC_PTR(addr)

// Marks the start of a loop that will repeat a certain number of times.
#define BEGIN_REPEAT(count) BC_B0H(0x05, count)

// Marks the end of a repeating loop.
#define END_REPEAT() BC_B(0x06)

// Also marks the end of a repeating loop, but continues executing commands
// following the loop on the same frame.
#define END_REPEAT_CONTINUE() BC_B(0x07)

// Marks the beginning of an infinite loop.
#define BEGIN_LOOP() BC_B(0x08)

// Marks the end of an infinite loop.
#define END_LOOP() BC_B(0x09)

// Exits the behavior script.
// Often used to end behavior scripts that do not contain an infinite loop.
#define BREAK() BC_B(0x0A)

// Exits the behavior script, unused.
#define BREAK_UNUSED() BC_B(0x0B)

// Executes a native game function.
#define CALL_NATIVE(func) BC_B(0x0C), BC_PTR(func)

// Adds a float to the specified field.
#define ADD_FLOAT(field, value) BC_BBH(0x0D, field, value)

// Sets the specified field to a float.
#define SET_FLOAT(field, value) BC_BBH(0x0E, field, value)

// Adds an integer to the specified field.
#define ADD_INT(field, value) BC_BBH(0x0F, field, value)

// Sets the specified field to an integer.
#define SET_INT(field, value) BC_BBH(0x10, field, value)

// Performs a bitwise OR with the specified field and the given integer.
// Usually used to set an object's flags.
#define OR_INT(field, value) BC_BBH(0x11, field, value)

// Performs a bit clear with the specified short. Unused in favor of the 32-bit
// version.
#define BIT_CLEAR(field, value) BC_BBH(0x12, field, value)

// TODO: this one needs a better name / labelling
// Gets a random short, right shifts it the specified amount and adds min to it,
// then sets the specified field to that value.
#define SET_INT_RAND_RSHIFT(field, min, rshift) \
  BC_BBH(0x13, field, min), BC_H(rshift)

// Sets the specified field to a random float in the given range.
#define SET_RANDOM_FLOAT(field, min, range) \
  BC_BBH(0x14, field, min), BC_H(range)

// Sets the specified field to a random integer in the given range.
#define SET_RANDOM_INT(field, min, range) BC_BBH(0x15, field, min), BC_H(range)

// Adds a random float in the given range to the specified field.
#define ADD_RANDOM_FLOAT(field, min, range) \
  BC_BBH(0x16, field, min), BC_H(range)

// TODO: better name (unused anyway)
// Gets a random short, right shifts it the specified amount and adds min to it,
// then adds the value to the specified field. Unused.
#define ADD_INT_RAND_RSHIFT(field, min, rshift) \
  BC_BBH(0x17, field, min), BC_H(rshift)

// No operation. Unused.
#define CMD_NOP_1(field) BC_BB(0x18, field)

// No operation. Unused.
#define CMD_NOP_2(field) BC_BB(0x19, field)

// No operation. Unused.
#define CMD_NOP_3(field) BC_BB(0x1A, field)

// Sets the current model ID of the object.
#define SET_MODEL(modelID) BC_B0H(0x1B, modelID)

// Spawns a child object with the specified model and behavior.
#define SPAWN_CHILD(modelID, behavior) \
  BC_B(0x1C), BC_W(modelID), BC_PTR(behavior)

// Exits the behavior script and despawns the object.
// Often used to end behavior scripts that do not contain an infinite loop.
#define DEACTIVATE() BC_B(0x1D)

// Finds the floor triangle directly under the object and moves the object down
// to it.
#define DROP_TO_FLOOR() BC_B(0x1E)

// Sets the destination float field to the sum of the values of the given float
// fields.
#define SUM_FLOAT(fieldDst, fieldSrc1, fieldSrc2) \
  BC_BBBB(0x1F, fieldDst, fieldSrc1, fieldSrc2)

// Sets the destination integer field to the sum of the values of the given
// integer fields. Unused.
#define SUM_INT(fieldDst, fieldSrc1, fieldSrc2) \
  BC_BBBB(0x20, fieldDst, fieldSrc1, fieldSrc2)

// Billboards the current object, making it always face the camera.
#define BILLBOARD() BC_B(0x21)

// Hides the current object.
#define HIDE() BC_B(0x22)

// Sets the size of the object's cylindrical hitbox.
#define SET_HITBOX(radius, height) BC_B(0x23), BC_HH(radius, height)

// No operation. Unused.
#define CMD_NOP_4(field, value) BC_BBH(0x24, field, value)

// Delays the behavior script for the number of frames given by the value of the
// specified field.
#define DELAY_VAR(field) BC_BB(0x25, field)

// Unused. Marks the start of a loop that will repeat a certain number of times.
// Uses a u8 as the argument, instead of a s16 like the other version does.
#define BEGIN_REPEAT_UNUSED(count) BC_BB(0x26, count)

// Loads the animations for the object. <field> is always set to oAnimations.
#define LOAD_ANIMATIONS(field, anims) BC_BB(0x27, field), BC_PTR(anims)

// Begins animation and sets the object's current animation index to the
// specified value.
#define ANIMATE(animIndex) BC_BB(0x28, animIndex)

// Spawns a child object with the specified model and behavior, plus a behavior
// param.
#define SPAWN_CHILD_WITH_PARAM(bhvParam, modelID, behavior) \
  BC_B0H(0x29, bhvParam), BC_W(modelID), BC_PTR(behavior)

// Loads collision data for the object.
#define LOAD_COLLISION_DATA(collisionData) BC_B(0x2A), BC_PTR(collisionData)

// Sets the size of the object's cylindrical hitbox, and applies a downwards
// offset.
#define SET_HITBOX_WITH_OFFSET(radius, height, downOffset) \
  BC_B(0x2B), BC_HH(radius, height), BC_H(downOffset)

// Spawns a new object with the specified model and behavior.
#define SPAWN_OBJ(modelID, behavior) BC_B(0x2C), BC_W(modelID), BC_PTR(behavior)

// Sets the home position of the object to its current position.
#define SET_HOME() BC_B(0x2D)

// Sets the size of the object's cylindrical hurtbox.
#define SET_HURTBOX(radius, height) BC_B(0x2E), BC_HH(radius, height)

// Sets the object's interaction type.
#define SET_INTERACT_TYPE(type) BC_B(0x2F), BC_W(type)

// Sets various parameters that the object uses for calculating physics.
#define SET_OBJ_PHYSICS(wallHitboxRadius,                         \
                        gravity,                                  \
                        bounciness,                               \
                        dragStrength,                             \
                        friction,                                 \
                        buoyancy,                                 \
                        unused1,                                  \
                        unused2)                                  \
  BC_B(0x30), BC_HH(wallHitboxRadius, gravity),                   \
      BC_HH(bounciness, dragStrength), BC_HH(friction, buoyancy), \
      BC_HH(unused1, unused2)

// Sets the object's interaction subtype. Unused.
#define SET_INTERACT_SUBTYPE(subtype) BC_B(0x31), BC_W(subtype)

// Sets the object's size to the specified percentage.
#define SCALE(unusedField, percent) BC_BBH(0x32, unusedField, percent)

// Performs a bit clear on the object's parent's field with the specified value.
// Used for clearing active particle flags fron Mario's object.
#define PARENT_BIT_CLEAR(field, flags) BC_BB(0x33, field), BC_W(flags)

// Animates an object using texture animation. <field> is always set to
// oAnimState.
#define ANIMATE_TEXTURE(field, rate) BC_BBH(0x34, field, rate)

// Disables rendering for the object.
#define DISABLE_RENDERING() BC_B(0x35)

// Unused. Sets the specified field to an integer. Wastes 4 bytes of space for
// no reason at all.
#define SET_INT_UNUSED(field, value) BC_BB(0x36, field), BC_HH(0, value)

// Spawns a water droplet with the given parameters.
#define SPAWN_WATER_DROPLET(dropletParams) BC_B(0x37), BC_PTR(dropletParams)