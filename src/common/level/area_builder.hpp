#pragma once
#include <functional>
#include <memory>

#include "common/scripts/dynamic_level_script_builder.hpp"
#include "common/scripts/i_level_script_builder.hpp"

#include "i_object_builder.hpp"
#include "object_builder_params.hpp"

class AreaBuilder : public ILevelScriptBuilder {
public:
  AreaBuilder(u8 area_index, const GeoLayout* geo_layout);
  AreaBuilder(const AreaBuilder& other) = delete;

  AreaBuilder& add_part(
      std::shared_ptr<IScriptPart<LevelScript>> part) override;

  AreaBuilder& add_script(LevelScript script) override;
  AreaBuilder& add_scripts(
      std::initializer_list<const LevelScript> scripts) override;
  AreaBuilder& add_scripts(const LevelScript* scripts,
                           int script_count) override;

  AreaBuilder& add_builder(
      std::shared_ptr<IScriptBuilder<LevelScript>> builder) override;

  AreaBuilder& add_object(std::shared_ptr<IObjectBuilder> object_builder,
                          const std::function<void(ObjectBuilderParams&)>&
                          params_callback);

  int size() const override;
  void build_into(LevelScript* dst, int& dst_pos) const override;
  std::weak_ptr<ValidationNode> get_cache_validation_node() override;

private:
  const std::unique_ptr<DynamicLevelScriptBuilder> entry_;
  const std::shared_ptr<DynamicLevelScriptBuilder> body_;
};
