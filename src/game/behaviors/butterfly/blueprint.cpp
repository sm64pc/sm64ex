#include "blueprint.hpp"

#include "common/level/wrapped_object_builder.hpp"
#include "include/behavior_macros.h"

#include "butterfly.hpp"

typedef WrappedObjectBuilder<Butterfly> WrappedButterflyBuilder;

std::shared_ptr<IObjectBuilder> get_butterfly_blueprint() {
  auto blueprint = std::make_shared<WrappedButterflyBuilder>(
      MODEL_BUTTERFLY,
      OBJ_LIST_DEFAULT);
  (*blueprint).add_scripts({
      OR_INT(oFlags,
             (OBJ_FLAG_SET_FACE_YAW_TO_MOVE_YAW |
               OBJ_FLAG_UPDATE_GFX_POS_AND_ANGLE)),
      LOAD_ANIMATIONS(oAnimations, butterfly_seg3_anims_030056B0),
      DROP_TO_FLOOR(),
      SET_FLOAT(oGraphYOffset, 5),
  });

  return blueprint;
}
