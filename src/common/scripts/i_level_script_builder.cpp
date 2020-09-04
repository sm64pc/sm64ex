#include "i_level_script_builder.hpp"

const LevelScript* ILevelScriptBuilder::get_entry_pointer(int& out_count) const {
  return build_impl(out_count);
}

const LevelScript* ILevelScriptBuilder::build_impl(int& out_count) const {
  const auto script_count = size();

  const auto dst = new LevelScript[script_count];
  auto dst_pos = 0;
  build_into(dst, dst_pos);

  out_count = script_count;
  return dst;
}
