#pragma once
#include "common/object/i_physical_object_wrapper.hpp"
#include "include/types.h"

enum class ButterflyState {
  RESTING,
  FOLLOW_MARIO,
  RETURN_HOME
};


class Butterfly : public IPhysicalObjectWrapper {
 public:
  explicit Butterfly(struct Object* wrapped_object)
      : IPhysicalObjectWrapper(wrapped_object) {}
  Butterfly(const Butterfly& other) = delete;

  void init() override;
  void tick() override;

private:
  void step(s32 speed);

  void tick_rest();
  void tick_follow_mario();
  void tick_return_home();

  ButterflyState state_ = ButterflyState::RESTING;
};
