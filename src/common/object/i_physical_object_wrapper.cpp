#include "i_physical_object_wrapper.hpp"

#include "include/object_fields.h"

IPhysicalObjectWrapper::IPhysicalObjectWrapper(struct Object* wrapped_object)
  : IObjectWrapper(wrapped_object),
    position_(&wrapped_object->oPosX,
              &wrapped_object->oPosY,
              &wrapped_object->oPosZ),
    home_(&wrapped_object->oHomeX,
          &wrapped_object->oHomeY,
          &wrapped_object->oHomeZ) {}
