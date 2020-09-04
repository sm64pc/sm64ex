#pragma once

#include "include/types.h"

class IObjectBuilder {
 public:
  IObjectBuilder() = default;
  IObjectBuilder(const IObjectBuilder& other) = delete;

  virtual u16 get_model_index() = 0;
  virtual const BehaviorScript* get_behavior_scripts() const = 0;
};
