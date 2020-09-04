#pragma once

#include "i_object_wrapper.hpp"
#include "position_wrapper.hpp"

class IPhysicalObjectWrapper : public IObjectWrapper {
  public:
    explicit IPhysicalObjectWrapper(struct Object* wrapped_object);

  protected:
    PositionWrapper position_;
    PositionWrapper home_;
};
