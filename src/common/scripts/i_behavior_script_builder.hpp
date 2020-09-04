#pragma once

#include <memory>

#include "include/types.h"
#include "util/unused.hpp"

#include "i_script_builder.hpp"

class IBehaviorScriptBuilder : public IScriptBuilder<BehaviorScript> {
 public:
  IBehaviorScriptBuilder() = default;
  IBehaviorScriptBuilder(const IBehaviorScriptBuilder& other) = delete;

  IBehaviorScriptBuilder& add_part(
      std::shared_ptr<IScriptPart<BehaviorScript>> part) override = 0;

  IBehaviorScriptBuilder& add_script(BehaviorScript script) override = 0;
  IBehaviorScriptBuilder& add_scripts(
      std::initializer_list<const BehaviorScript> scripts) override = 0;
  IBehaviorScriptBuilder& add_scripts(const BehaviorScript* scripts,
                                   int script_count) override = 0;

  IBehaviorScriptBuilder& add_builder(
      std::shared_ptr<IScriptBuilder<BehaviorScript>> other) override = 0;

  int size() const override = 0;

  void build_into(LevelScript* dst, int& dst_count) const override = 0;

  const BehaviorScript* get_entry_pointer(
      int& out_count = unused_int) const override = 0;
  std::weak_ptr<ValidationNode> get_cache_validation_node() override = 0;

 protected:
  const BehaviorScript* build_impl(int& out_count) const;
};
