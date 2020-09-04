#include "level_script_part.hpp"

#include "constants.hpp"
#include "level_commands.h"

int LevelScriptPart::size() const {
  switch (type) {
    case LevelScriptPartType::CALL:
      return 2;

    case LevelScriptPartType::JUMP_TO_TOP_OF_THIS_BUILDER:
      return JUMP_COUNT;
    case LevelScriptPartType::JUMP_LINK_TO_ADDRESS:
    case LevelScriptPartType::JUMP_LINK_TO_BUILDER:
      return JUMP_LINK_COUNT;
    case LevelScriptPartType::JUMP_IF_EQUAL_TO_ADDRESS:
    case LevelScriptPartType::JUMP_IF_EQUAL_TO_BUILDER:
      return JUMP_IF_COUNT;

    case LevelScriptPartType::EXECUTE_ADDRESS:
    case LevelScriptPartType::EXECUTE_BUILDER:
      return EXECUTE_COUNT;
    case LevelScriptPartType::EXIT_AND_EXECUTE_BUILDER:
      return EXIT_AND_EXECUTE_COUNT;

    default:
      return 0;
  }
}

void LevelScriptPart::build_into(LevelScript* dst, int& dst_pos) const {
  switch (type) {
    case LevelScriptPartType::CALL:
      append_scripts(dst, dst_pos, {CALL(callback_arg, callback)});
      break;

    case LevelScriptPartType::JUMP_TO_TOP_OF_THIS_BUILDER:
      append_jump_to_address(dst, dst_pos, dst + jump_offset);
      break;
    case LevelScriptPartType::JUMP_LINK_TO_ADDRESS:
      append_jump_link_to_address(dst, dst_pos, address);
      break;
    case LevelScriptPartType::JUMP_LINK_TO_BUILDER: {
      const auto inner_inner_scripts = builder->get_entry_pointer();
      append_jump_link_to_address(dst, dst_pos, inner_inner_scripts);
      break;
    }
    case LevelScriptPartType::JUMP_IF_EQUAL_TO_ADDRESS:
      append_jump_if_equal_to_address(dst, dst_pos, value, address);
      break;
    case LevelScriptPartType::JUMP_IF_EQUAL_TO_BUILDER: {
      const auto inner_inner_scripts = builder->get_entry_pointer();
      append_jump_if_equal_to_address(
          dst, dst_pos, value, inner_inner_scripts);
      break;
    }

    case LevelScriptPartType::EXECUTE_ADDRESS:
      append_execute(dst,
                     dst_pos,
                     segment,
                     segment_start,
                     segment_end,
                     address);
      break;
    case LevelScriptPartType::EXECUTE_BUILDER: {
      const auto inner_inner_scripts = builder->get_entry_pointer();
      append_execute(dst,
                     dst_pos,
                     segment,
                     segment_start,
                     segment_end,
                     inner_inner_scripts);
      break;
    }
    case LevelScriptPartType::EXIT_AND_EXECUTE_BUILDER: {
      const auto inner_inner_scripts = builder->get_entry_pointer();
      append_exit_and_execute(dst,
                              dst_pos,
                              segment,
                              segment_start,
                              segment_end,
                              inner_inner_scripts);
      break;
    }
  }
}