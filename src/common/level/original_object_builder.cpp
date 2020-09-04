#include "original_object_builder.hpp"


OriginalObjectBuilder::OriginalObjectBuilder(u16 model_index,
                                             const BehaviorScript*
                                             behavior_scripts)
  : model_index(model_index),
    behavior_scripts(behavior_scripts) {}

u16 OriginalObjectBuilder::get_model_index() { return model_index; }

const BehaviorScript* OriginalObjectBuilder::get_behavior_scripts() const {
  return behavior_scripts;
}
