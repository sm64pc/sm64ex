#pragma once
#include "common/object/i_object_wrapper.hpp"

enum class GrindelOrThwompState {
  RISING, // oAction #0
  WAITING_TO_FALL, // oAction #1
  FALLING, // oAction #2
  SHAKING_SCREEN, // oAction #3,
  WAITING_TO_RISE, // oAction #4
};

/**
 * Wrapped instance of a Grindel/Thwomp.
 */
class GrindelOrThwomp : public IObjectWrapper {
public:
  GrindelOrThwomp(struct Object *wrapped_object);

  void tick() override;

private:
};

extern void bhv_grindel_thwomp_loop();