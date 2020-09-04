#ifndef LEVEL_SCRIPT_H
#define LEVEL_SCRIPT_H

#include <PR/ultratypes.h>

#include "util/unused.hpp"

struct LevelCommand;

extern const LevelScript* get_level_script_entry(int& out_count = unused_int);

struct LevelCommand* level_script_execute(struct LevelCommand* cmd);

#endif // LEVEL_SCRIPT_H
