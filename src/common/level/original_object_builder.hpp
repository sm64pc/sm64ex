#pragma once

#include "i_object_builder.hpp"
#include "include/types.h"

class OriginalObjectBuilder : public IObjectBuilder {
public:
  OriginalObjectBuilder(u16 model_index, const BehaviorScript* scripts);
  OriginalObjectBuilder(const OriginalObjectBuilder& other) = delete;

  u16 get_model_index() override;
  const BehaviorScript* get_behavior_scripts() const override;

private:
  const u16 model_index;
  const BehaviorScript* behavior_scripts;
};
