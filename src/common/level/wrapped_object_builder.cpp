#include "wrapped_object_builder.hpp"

#include "common/scripts/util.hpp"
#include "game/behaviors/butterfly/butterfly.hpp"

#include "behavior_macros.h"

// TODO: Remove this call by merging the WrappedObjectBuilder hpp/cpp files.
template class WrappedObjectBuilder<Butterfly>;

template <class TObjectWrapper>
WrappedObjectBuilder<TObjectWrapper>::WrappedObjectBuilder(
    u16 model_index, ObjectList object_type)
    : model_index(model_index) {
  add_script(BEGIN(object_type));
}

template <class TObjectWrapper>
u16 WrappedObjectBuilder<TObjectWrapper>::get_model_index() {
  return model_index;
}

template <class TObjectWrapper>
void bhv_wrapper_init() {
  IObjectWrapper::get_or_create_wrapper_for<TObjectWrapper>(gCurrentObject)->init();
}

void bhv_wrapper_loop() {
  IObjectWrapper::get_wrapper_for(gCurrentObject)->tick();
}

template <class TObjectWrapper>
const BehaviorScript* WrappedObjectBuilder<TObjectWrapper>::get_entry_pointer(
    int& out_count) const {
  if (cached_behavior_scripts != nullptr) {
    out_count = size();
    return cached_behavior_scripts;
  }

  // TODO: Move this into a wrapper around body_.
  auto unconst_this = const_cast<WrappedObjectBuilder<TObjectWrapper>*>(this);
  unconst_this->add_scripts({
      CALL_NATIVE(bhv_wrapper_init<TObjectWrapper>),
      BEGIN_LOOP(),
      CALL_NATIVE(bhv_wrapper_loop),
      END_LOOP(),
  });

  return build_impl(out_count);
}

template <class TObjectWrapper>
WrappedObjectBuilder<TObjectWrapper>&
WrappedObjectBuilder<TObjectWrapper>::add_part(
    std::shared_ptr<IScriptPart<BehaviorScript>> part) {
  parts_.push_back(std::move(part));
  return *this;
}

template <class TObjectWrapper>
WrappedObjectBuilder<TObjectWrapper>&
WrappedObjectBuilder<TObjectWrapper>::add_script(BehaviorScript script) {
  return add_part(std::make_shared<SingleScriptPart<BehaviorScript>>(script));
}

template <class TObjectWrapper>
WrappedObjectBuilder<TObjectWrapper>&
WrappedObjectBuilder<TObjectWrapper>::add_scripts(
    std::initializer_list<const BehaviorScript> scripts) {
  return add_part(
      std::make_shared<MultipleScriptPart<BehaviorScript>>(scripts));
}

template <class TObjectWrapper>
WrappedObjectBuilder<TObjectWrapper>&
WrappedObjectBuilder<TObjectWrapper>::add_scripts(const BehaviorScript* scripts,
                                                  int script_count) {
  return add_part(std::make_shared<MultipleScriptPart<BehaviorScript>>(
      scripts, script_count));
}

template <class TObjectWrapper>
WrappedObjectBuilder<TObjectWrapper>&
WrappedObjectBuilder<TObjectWrapper>::add_builder(
    std::shared_ptr<IScriptBuilder<BehaviorScript>> builder) {
  return add_part(std::make_shared<BuilderScriptPart<BehaviorScript>>(builder));
}

template <class TObjectWrapper>
int WrappedObjectBuilder<TObjectWrapper>::size() const {
  auto total_size = 0;
  for (const auto& part : parts_) {
    total_size += part->size();
  }
  return total_size;
}

template <class TObjectWrapper>
void WrappedObjectBuilder<TObjectWrapper>::build_into(LevelScript* dst,
                                                      int& dst_pos) const {
  for (const auto& part : parts_) {
    part->build_into(dst, dst_pos);
  }
}

template <class TObjectWrapper>
std::weak_ptr<ValidationNode>
WrappedObjectBuilder<TObjectWrapper>::get_cache_validation_node() {
  return cache_validation_impl_;
}
