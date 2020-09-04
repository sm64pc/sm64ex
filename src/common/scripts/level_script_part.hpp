#pragma once
#include "i_script_builder.hpp"
#include "i_script_part.hpp"

class ILevelScriptPart : public IScriptPart<LevelScript> {
public:
  int size() const override = 0;
  void build_into(LevelScript* dst, int& dst_pos) const override = 0;
};

enum class LevelScriptPartType {
  CALL,

  JUMP_TO_TOP_OF_THIS_BUILDER,
  JUMP_LINK_TO_ADDRESS,
  JUMP_LINK_TO_BUILDER,
  JUMP_IF_EQUAL_TO_ADDRESS,
  JUMP_IF_EQUAL_TO_BUILDER,

  EXECUTE_ADDRESS,
  EXECUTE_BUILDER,
  EXIT_AND_EXECUTE_BUILDER,
};

class LevelScriptPart : public ILevelScriptPart {
public:
  int size() const override;
  void build_into(LevelScript* dst, int& dst_pos) const override;

  LevelScriptPartType type;
  void (*callback)(void);
  uintptr_t callback_arg;
  u32 value;
  u8 segment;
  const u8* segment_start;
  const u8* segment_end;
  std::shared_ptr<IScriptBuilder<LevelScript>> builder;
  const LevelScript* address;
  u8 jump_offset;
};
