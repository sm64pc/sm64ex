#pragma once
#include <initializer_list>
#include <memory>
#include <vector>

#include "common/scripts/i_behavior_script_builder.hpp"
#include "game/object_list_processor.h"

#include "i_object_builder.hpp"

// TODO: Memory leaking will be a problem with the behavior scripts.
// TODO: Will not update if changes are made after first build.
// TODO: Each version of the build should maintain the same pointer location.
template <class TObjectWrapper>
class WrappedObjectBuilder
    : public IBehaviorScriptBuilder, public IObjectBuilder {
public:
  WrappedObjectBuilder(u16 model_index, ObjectList object_type);
  WrappedObjectBuilder(const WrappedObjectBuilder<TObjectWrapper>& other) = delete;

  u16 get_model_index() override;

  const BehaviorScript* get_behavior_scripts() const override {
    return get_entry_pointer();
  }

  const BehaviorScript* get_entry_pointer(int& out_count = unused_int) const
  override;

  WrappedObjectBuilder& add_part(
      std::shared_ptr<IScriptPart<BehaviorScript>> part) override;

  WrappedObjectBuilder& add_script(BehaviorScript script) override;
  WrappedObjectBuilder& add_scripts(
      std::initializer_list<const BehaviorScript> scripts) override;
  WrappedObjectBuilder& add_scripts(const BehaviorScript* scripts,
                                    int script_count) override;

  WrappedObjectBuilder& add_builder(
      std::shared_ptr<IScriptBuilder<BehaviorScript>> builder) override;

  int size() const override;
  void build_into(LevelScript* dst, int& dst_count) const override;
  std::weak_ptr<ValidationNode> get_cache_validation_node() override;

private:
  const u16 model_index;
  std::vector<std::shared_ptr<IScriptPart<BehaviorScript>>> parts_;

  std::shared_ptr<ValidationNode> cache_validation_impl_;

  const BehaviorScript* cached_behavior_scripts = nullptr;
};
