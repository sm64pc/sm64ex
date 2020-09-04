#include "thwomp.hpp"
#include "audio_defines.h"
#include "common/object/i_object_wrapper.hpp"
#include "engine/behavior_script.h"
#include "game/behavior_actions.h"
#include "game/object_helpers.h"
#include "game/object_list_processor.h"
#include "game/spawn_sound.h"
#include "object_fields.h"

#define o gCurrentObject

void grindel_thwomp_act_4(void) {
  if (o->oTimer == 0)
    o->oThwompRandomTimer = random_float() * 10.0f + 20.0f;
  if (o->oTimer > o->oThwompRandomTimer)
    o->oAction = 0;
}

void grindel_thwomp_act_2(void) {
  o->oVelY += -4.0f;
  o->oPosY += o->oVelY;
  if (o->oPosY < o->oHomeY) {
    o->oPosY = o->oHomeY;
    o->oVelY = 0;
    o->oAction = 3;
  }
}

void grindel_thwomp_act_3(void) {
  if (o->oTimer == 0)
    if (o->oDistanceToMario < 1500.0f) {
      cur_obj_shake_screen(SHAKE_POS_SMALL);
      cur_obj_play_sound_2(SOUND_OBJ_THWOMP);
    }
  if (o->oTimer > 9)
    o->oAction = 4;
}

void grindel_thwomp_act_1(void) {
  if (o->oTimer == 0)
    o->oThwompRandomTimer = random_float() * 30.0f + 10.0f;
  if (o->oTimer > o->oThwompRandomTimer)
    o->oAction = 2;
}

void grindel_thwomp_act_0(void) {
  if (o->oBehParams2ndByte + 40 < o->oTimer) {
    o->oAction = 1;
    o->oPosY += 5.0f;
  } else
    o->oPosY += 10.0f;
}

void (*sGrindelThwompActions[])(void) = { grindel_thwomp_act_0, grindel_thwomp_act_1,
                                          grindel_thwomp_act_2, grindel_thwomp_act_3,
                                          grindel_thwomp_act_4 };

GrindelOrThwomp::
GrindelOrThwomp(struct Object* wrapped_object) : IObjectWrapper(
  wrapped_object) { }

void GrindelOrThwomp::tick() {
  cur_obj_call_action_function(sGrindelThwompActions);
}

void bhv_grindel_thwomp_loop() {
  auto grindel_or_thwomp =
      IObjectWrapper::get_or_create_wrapper_for<GrindelOrThwomp>(o);
  grindel_or_thwomp->tick();
}
