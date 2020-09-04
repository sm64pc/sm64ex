#pragma once

#include <map>
#include <memory>

#include "include/types.h"

/**
 * Interface for "object wrappers", which allows us to reframe Super Mario 64's
 * C-style instance system in an C++ way.
 */
// TODO: Move these static methods to a management service.
class IObjectWrapper {
 public:
  // TODO: Remote this method, store the wrapper on the object.
  template <class TWrapper>
  static TWrapper* get_or_create_wrapper_for(struct Object* wrapped_object) {
    auto& wrapper_ptr = wrapped_object->wrapper;

    auto wrapper = (TWrapper*)wrapper_ptr.get();
    if (wrapper == nullptr) {
      wrapper = new TWrapper(wrapped_object);
      wrapper_ptr.reset(wrapper);
    }

    return wrapper;
  }

  static IObjectWrapper* get_wrapper_for(struct Object* wrapped_object);
  // TODO: Don't make this such an easily accessed public method...
  static void destroy_wrapper_for(struct Object* wrapped_object);

  // TODO: Limit construction to just a factory method.
  explicit IObjectWrapper(struct Object* wrapped_object);

  virtual void init() {}
  virtual void tick() {}

 protected:
  struct Object* wrapped_object;
};
