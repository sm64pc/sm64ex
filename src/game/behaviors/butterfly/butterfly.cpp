#include "butterfly.hpp"

#include "engine/behavior_script.h"
#include "engine/math_util.h"
#include "game/object_helpers.h"
#include "game/object_list_processor.h"
#include "game/obj_behaviors.h"
#include "include/object_fields.h"

#define o gCurrentObject

const s32 kStartFollowingMarioDistance = 1000;
const s32 kStartReturningHomeDistance = 1200;

const s32 kFollowMarioSpeed = 7;
const s32 kReturnHomeSpeed = 7;

// TODO: Move this to a common file.
s32 is_point_within_radius_of_mario_(f32 x, f32 y, f32 z, s32 dist) {
  const auto mGfxX = gMarioObject->header.gfx.pos[0];
  const auto mGfxY = gMarioObject->header.gfx.pos[1];
  const auto mGfxZ = gMarioObject->header.gfx.pos[2];

  if ((x - mGfxX) * (x - mGfxX) + (y - mGfxY) * (y - mGfxY) +
      (z - mGfxZ) * (z - mGfxZ) <
      (f32)(dist * dist)) { return TRUE; }

  return FALSE;
}

// TODO: Move this to a common file.
void set_object_visibility_(struct Object* obj, s32 dist) {
  const auto objX = obj->oPosX;
  const auto objY = obj->oPosY;
  const auto objZ = obj->oPosZ;

  if (is_point_within_radius_of_mario_(objX, objY, objZ, dist) == TRUE) {
    obj->header.gfx.node.flags &= ~GRAPH_RENDER_INVISIBLE;
  } else { obj->header.gfx.node.flags |= GRAPH_RENDER_INVISIBLE; }
}

void Butterfly::init() {
  cur_obj_init_animation(1);

  o->oButterflyYPhase = random_float() * 100.0f;
  o->header.gfx.unk38.animFrame = random_float() * 7.0f;
  home_.set(position_);
}

void Butterfly::tick() {
  switch (state_) {
    case ButterflyState::RESTING:
      tick_rest();
      break;

    case ButterflyState::FOLLOW_MARIO:
      tick_follow_mario();
      break;

    case ButterflyState::RETURN_HOME:
      tick_return_home();
      break;
  }

  set_object_visibility_(o, 3000);
}

// sp28 = speed

void Butterfly::step(s32 speed) {
  const auto yaw = o->oMoveAngleYaw;
  const auto pitch = o->oMoveAnglePitch;
  const auto y_phase = o->oButterflyYPhase;

  o->oVelX = sins(yaw) * (f32)speed;
  o->oVelY = sins(pitch) * (f32)speed;
  o->oVelZ = coss(yaw) * (f32)speed;

  o->oPosX += o->oVelX;
  o->oPosZ += o->oVelZ;

  if (state_ == ButterflyState::FOLLOW_MARIO) {
    o->oPosY -= o->oVelY + coss((s32)(y_phase * 655.36)) * 20.0f / 4;
  } else { o->oPosY -= o->oVelY; }

  struct FloorGeometry* sp24;
  const auto floor_y =
      find_floor_height_and_data(o->oPosX, o->oPosY, o->oPosZ, &sp24);

  if (o->oPosY < floor_y + 2.0f) {
    o->oPosY = floor_y + 2.0f;
  }

  o->oButterflyYPhase++;
  if (o->oButterflyYPhase >= 101) { o->oButterflyYPhase = 0; }
}

void butterfly_calculate_angle_(void) {
  gMarioObject->oPosX += 5 * o->oButterflyYPhase / 4;
  gMarioObject->oPosZ += 5 * o->oButterflyYPhase / 4;
  obj_turn_toward_object(o, gMarioObject, 16, 0x300);
  gMarioObject->oPosX -= 5 * o->oButterflyYPhase / 4;
  gMarioObject->oPosZ -= 5 * o->oButterflyYPhase / 4;

  gMarioObject->oPosY += (5 * o->oButterflyYPhase + 0x100) / 4;
  obj_turn_toward_object(o, gMarioObject, 15, 0x500);
  gMarioObject->oPosY -= (5 * o->oButterflyYPhase + 0x100) / 4;
}

void Butterfly::tick_rest() {
  if (is_point_within_radius_of_mario_(
      o->oPosX,
      o->oPosY,
      o->oPosZ,
      kStartFollowingMarioDistance)) {
    cur_obj_init_animation(0);

    state_ = ButterflyState::FOLLOW_MARIO;
    o->oMoveAngleYaw = gMarioObject->header.gfx.angle[1];
  }
}

void Butterfly::tick_follow_mario() {
  butterfly_calculate_angle_();

  butterfly_step(kFollowMarioSpeed);

  if (!is_point_within_radius_of_mario_(
      o->oHomeX,
      o->oHomeY,
      o->oHomeZ,
      kStartReturningHomeDistance)) { state_ = ButterflyState::RETURN_HOME; }
}

void Butterfly::tick_return_home() {
  s16 h_angle_to_home;
  s16 v_angle_to_home;
  position_.calc_angle_to(home_, h_angle_to_home, v_angle_to_home);

  o->oMoveAngleYaw =
      approach_s16_symmetric(o->oMoveAngleYaw, h_angle_to_home, 0x800);
  o->oMoveAnglePitch =
      approach_s16_symmetric(o->oMoveAnglePitch, v_angle_to_home, 0x50);

  butterfly_step(kReturnHomeSpeed);

  if (position_.calc_sqr_distance_to(home_) < 144.0f) {
    cur_obj_init_animation(1);

    state_ = ButterflyState::RESTING;
    position_.set(home_);
  }
}
