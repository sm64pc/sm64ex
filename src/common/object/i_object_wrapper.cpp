#include "i_object_wrapper.hpp"

IObjectWrapper* IObjectWrapper::get_wrapper_for(struct Object* wrapped_object) {
  return (IObjectWrapper*) wrapped_object->wrapper.get();
}

// TODO: Don't make this such an easily accessed public method...
void IObjectWrapper::destroy_wrapper_for(struct Object* wrapped_object) {
  wrapped_object->wrapper.reset();
}

IObjectWrapper::IObjectWrapper(struct Object* wrapped_object) {
  this->wrapped_object = wrapped_object;
}
