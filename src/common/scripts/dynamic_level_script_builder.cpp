#include "dynamic_level_script_builder.hpp"

#include "include/level_commands.h"

#include "util.hpp"

DynamicLevelScriptBuilder& DynamicLevelScriptBuilder::add_part(
    std::shared_ptr<IScriptPart<LevelScript>> part) {
  parts_.push_back(std::move(part));
  invalidate_cache();

  return *this;
}

DynamicLevelScriptBuilder& DynamicLevelScriptBuilder::add_script(
    LevelScript script) {
  return add_part(std::make_shared<SingleScriptPart<LevelScript>>(script));
}

DynamicLevelScriptBuilder& DynamicLevelScriptBuilder::add_scripts(
    std::initializer_list<const LevelScript> scripts) {
  return add_part(
      std::make_shared<MultipleScriptPart<LevelScript>>(scripts));
}

DynamicLevelScriptBuilder& DynamicLevelScriptBuilder::add_scripts(
    const LevelScript* scripts,
    int script_count) {
  return add_part(
      std::make_shared<MultipleScriptPart<LevelScript>>(
          scripts,
          script_count));
}

DynamicLevelScriptBuilder& DynamicLevelScriptBuilder::add_builder(
    std::shared_ptr<IScriptBuilder<LevelScript>> builder) {
  cache_validation_impl_->depend_on(builder->get_cache_validation_node());
  return add_part(
      std::make_shared<BuilderScriptPart<LevelScript>>(builder));
}

DynamicLevelScriptBuilder& DynamicLevelScriptBuilder::add_call(
    void (*callback)(void)) {
  auto part = new LevelScriptPart();
  part->type = LevelScriptPartType::CALL;
  part->callback = callback;

  return add_part(std::shared_ptr<LevelScriptPart>(part));
}

DynamicLevelScriptBuilder& DynamicLevelScriptBuilder::add_jump_link(
    const LevelScript* address) {
  auto part = new LevelScriptPart();
  part->type = LevelScriptPartType::JUMP_LINK_TO_ADDRESS;
  part->address = address;

  return add_part(std::shared_ptr<LevelScriptPart>(part));
}

DynamicLevelScriptBuilder& DynamicLevelScriptBuilder::add_jump_link(
    std::shared_ptr<ILevelScriptBuilder> builder) {
  auto part = new LevelScriptPart();
  part->type = LevelScriptPartType::JUMP_LINK_TO_BUILDER;
  part->builder = std::move(builder);

  return add_part(std::shared_ptr<LevelScriptPart>(part));
}

DynamicLevelScriptBuilder& DynamicLevelScriptBuilder::add_jump_if_equal(
    u32 value,
    const LevelScript* address) {
  auto part = new LevelScriptPart();
  part->type = LevelScriptPartType::JUMP_IF_EQUAL_TO_ADDRESS;
  part->value = value;
  part->address = address;

  return add_part(std::shared_ptr<LevelScriptPart>(part));
}

DynamicLevelScriptBuilder& DynamicLevelScriptBuilder::add_jump_if_equal(
    u32 value,
    std::shared_ptr<ILevelScriptBuilder> builder) {
  auto part = new LevelScriptPart();
  part->type = LevelScriptPartType::JUMP_IF_EQUAL_TO_BUILDER;
  part->value = value;
  part->builder = std::move(builder);

  return add_part(std::shared_ptr<LevelScriptPart>(part));
}

DynamicLevelScriptBuilder& DynamicLevelScriptBuilder::add_execute(
    u8 segment,
    const u8* segment_start,
    const u8* segment_end,
    const LevelScript* address) {
  auto part = new LevelScriptPart();
  part->type = LevelScriptPartType::EXECUTE_ADDRESS;
  part->segment = segment;
  part->segment_start = segment_start;
  part->segment_end = segment_end;
  part->address = address;

  return add_part(std::shared_ptr<LevelScriptPart>(part));
}

DynamicLevelScriptBuilder& DynamicLevelScriptBuilder::add_execute(
    u8 segment,
    const u8* segment_start,
    const u8* segment_end,
    std::shared_ptr<ILevelScriptBuilder> builder) {
  auto part = new LevelScriptPart();
  part->type = LevelScriptPartType::EXECUTE_BUILDER;
  part->segment = segment;
  part->segment_start = segment_start;
  part->segment_end = segment_end;
  part->builder = std::move(builder);

  return add_part(std::shared_ptr<LevelScriptPart>(part));
}

DynamicLevelScriptBuilder& DynamicLevelScriptBuilder::add_exit_and_execute(
    u8 segment,
    const u8* segment_start,
    const u8* segment_end,
    std::shared_ptr<ILevelScriptBuilder> builder) {
  auto part = new LevelScriptPart();
  part->type = LevelScriptPartType::EXIT_AND_EXECUTE_BUILDER;
  part->segment = segment;
  part->segment_start = segment_start;
  part->segment_end = segment_end;
  part->builder = std::move(builder);

  return add_part(std::shared_ptr<LevelScriptPart>(part));
}

DynamicLevelScriptBuilder&
DynamicLevelScriptBuilder::add_jump_to_top_of_this_builder(u8 jump_offset) {
  auto part = new LevelScriptPart();
  part->type = LevelScriptPartType::JUMP_TO_TOP_OF_THIS_BUILDER;
  part->jump_offset = jump_offset;

  return add_part(std::shared_ptr<LevelScriptPart>(part));
}

int DynamicLevelScriptBuilder::size() const {
  auto total_size = 0;
  for (const auto& part : parts_) { total_size += part->size(); }
  return total_size;
}

void DynamicLevelScriptBuilder::build_into(LevelScript* dst, int& dst_pos) const {
  for (const auto& part : parts_) { part->build_into(dst, dst_pos); }
}

std::weak_ptr<ValidationNode> DynamicLevelScriptBuilder::
get_cache_validation_node() { return cache_validation_impl_; }


bool DynamicLevelScriptBuilder::is_cache_valid() const {
  return cache_validation_impl_->is_valid();
}

void DynamicLevelScriptBuilder::invalidate_cache() {
  cache_validation_impl_->invalidate();
}
