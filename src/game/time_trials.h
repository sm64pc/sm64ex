#ifndef TIME_TRIALS_H
#define TIME_TRIALS_H

#include <stdbool.h>
#include "types.h"

#define TIME_TRIALS time_trials_enabled()

extern Gfx *gTimeTableDisplayListHead;
bool time_trials_enabled();
void time_trials_update(bool isPaused);
bool time_trials_render_time_table(s8 *index);
void time_trials_render_star_select_time(s32 starIndex);

#endif // TIME_TRIALS_H
