#include "i_behavior_script_builder.hpp"

const BehaviorScript* IBehaviorScriptBuilder::get_entry_pointer(
    int& out_count) const {
  return build_impl(out_count);
}

const BehaviorScript* IBehaviorScriptBuilder::build_impl(int& out_count) const {
  const auto script_count = size();

  const auto dst = new BehaviorScript[script_count];
  auto dst_pos = 0;
  build_into(dst, dst_pos);

  out_count = script_count;
  return dst;
}
