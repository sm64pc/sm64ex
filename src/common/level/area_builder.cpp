#include "area_builder.hpp"

#include "include/level_commands.h"

AreaBuilder::AreaBuilder(u8 area_index, const GeoLayout* geo_layout)
    : entry_(std::make_unique<DynamicLevelScriptBuilder>()),
      body_(std::make_shared<DynamicLevelScriptBuilder>()) {
  (*entry_)
      .add_scripts({AREA(area_index, geo_layout)})
      .add_builder(body_)
      .add_script(END_AREA());
}

AreaBuilder& AreaBuilder::add_part(
    std::shared_ptr<IScriptPart<LevelScript>> part) {
  body_->add_part(part);
  return *this;
}

AreaBuilder& AreaBuilder::add_script(LevelScript script) {
  body_->add_script(script);
  return *this;
}

AreaBuilder& AreaBuilder::add_scripts(
    std::initializer_list<const LevelScript> scripts) {
  body_->add_scripts(scripts);
  return *this;
}

AreaBuilder& AreaBuilder::add_scripts(const LevelScript* scripts,
                                      int script_count) {
  body_->add_scripts(scripts, script_count);
  return *this;
}

AreaBuilder& AreaBuilder::add_builder(
    std::shared_ptr<IScriptBuilder<LevelScript>> builder) {
  body_->add_builder(builder);
  return *this;
}

AreaBuilder& AreaBuilder::add_object(
    std::shared_ptr<IObjectBuilder> object_builder,
    const std::function<void(ObjectBuilderParams&)>& params_callback) {
  auto params = ObjectBuilderParams();
  params_callback(params);

  const auto model_index = object_builder->get_model_index();
  const auto x = params.x;
  const auto y = params.y;
  const auto z = params.z;
  const auto x_angle = params.x_angle;
  const auto y_angle = params.y_angle;
  const auto z_angle = params.z_angle;
  const auto bhv_param = params.beh_param;
  const auto bhv_scripts = object_builder->get_behavior_scripts();

  body_->add_scripts({OBJECT(model_index,
                             x,
                             y,
                             z,
                             x_angle,
                             y_angle,
                             z_angle,
                             bhv_param,
                             bhv_scripts)});

  return *this;
}

int AreaBuilder::size() const { return entry_->size(); }

void AreaBuilder::build_into(LevelScript* dst, int& dst_pos) const {
  entry_->build_into(dst, dst_pos);
}

std::weak_ptr<ValidationNode> AreaBuilder::get_cache_validation_node() {
  return body_->get_cache_validation_node();
}