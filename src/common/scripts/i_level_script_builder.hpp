#pragma once

#include <memory>

#include "include/types.h"
#include "util/unused.hpp"

#include "i_script_builder.hpp"

class ILevelScriptBuilder : public IScriptBuilder<LevelScript> {
public:
  ILevelScriptBuilder() = default;
  ILevelScriptBuilder(const ILevelScriptBuilder& other) = delete;

  ILevelScriptBuilder& add_part(
      std::shared_ptr<IScriptPart<LevelScript>> part) override = 0;

  ILevelScriptBuilder& add_script(LevelScript script) override = 0;
  ILevelScriptBuilder& add_scripts(
      std::initializer_list<const LevelScript> scripts) override = 0;
  ILevelScriptBuilder& add_scripts(const LevelScript* scripts,
                                   int script_count) override = 0;

  ILevelScriptBuilder& add_builder(
      std::shared_ptr<IScriptBuilder<LevelScript>> other) override = 0;

  int size() const override = 0;

  void build_into(LevelScript* dst, int& dst_count) const override = 0;

  const LevelScript* get_entry_pointer(int& out_count = unused_int) const
  override;
  std::weak_ptr<ValidationNode> get_cache_validation_node() override = 0;

 protected:
  const LevelScript* build_impl(int& out_count) const;
};
