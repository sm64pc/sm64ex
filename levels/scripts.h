#ifndef SCRIPTS_H
#define SCRIPTS_H

#include <memory>
#include "common/scripts/dynamic_level_script_builder.hpp"
#include "types.h"

extern std::shared_ptr<DynamicLevelScriptBuilder> get_script_exec_level_table();

#define STUB_LEVEL(_0, _1, _2, _3, _4, _5, _6, _7, _8)

#define DEFINE_LEVEL(_0, _1, _2, folder, _4, _5, _6, _7, _8, _9, _10) \
  extern std::shared_ptr<DynamicLevelScriptBuilder> get_script_exec_##folder();
#include "processed_level_defines.h"
#undef DEFINE_LEVEL

#define DEFINE_LEVEL(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10) +3
const int script_exec_level_table_size_ = 2
  #include "unprocessed_level_defines.h"
;
#undef DEFINE_LEVEL

#define DEFINE_LEVEL(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10) +3
extern const LevelScript script_exec_level_table_[2
  #include "unprocessed_level_defines.h"
];
#undef DEFINE_LEVEL

#define DEFINE_LEVEL(_0, _1, _2, folder, _4, _5, _6, _7, _8, _9, _10)                                  \
  extern const LevelScript script_exec_##folder[4 + 1];
#include "unprocessed_level_defines.h"
#undef DEFINE_LEVEL

#undef STUB_LEVEL

extern const LevelScript script_L1[4];
extern const LevelScript script_L2[4];
extern const LevelScript script_L3[4];
extern const LevelScript script_L4[4];
extern const LevelScript script_L5[4];

// scripts
extern LevelScript level_main_scripts_entry[];
extern const LevelScript* get_level_main_scripts_entry();
extern const LevelScript script_func_global_1[];
extern const LevelScript script_func_global_2[];
extern const LevelScript script_func_global_3[];
extern const LevelScript script_func_global_4[];
extern const LevelScript script_func_global_5[];
extern const LevelScript script_func_global_6[];
extern const LevelScript script_func_global_7[];
extern const LevelScript script_func_global_8[];
extern const LevelScript script_func_global_9[];
extern const LevelScript script_func_global_10[];
extern const LevelScript script_func_global_11[];
extern const LevelScript script_func_global_12[];
extern const LevelScript script_func_global_13[];
extern const LevelScript script_func_global_14[];
extern const LevelScript script_func_global_15[];
extern const LevelScript script_func_global_16[];
extern const LevelScript script_func_global_17[];
extern const LevelScript script_func_global_18[];

#endif
