#pragma once

#include <initializer_list>

#include "include/types.h"

template <typename TScript>
extern void append_script(TScript* dst, int& dst_pos,
                          TScript src);
template <typename TScript>
extern void append_scripts(TScript* dst, int& dst_pos,
                           const TScript* src,
                           const int src_count);
template <typename TScript>
extern void append_scripts(TScript* dst, int& dst_pos,
                           std::initializer_list<const TScript> src);

extern void append_jump_to_address(LevelScript* dst, int& dst_pos,
                                   const LevelScript* address);
extern void append_jump_link_to_address(LevelScript* dst, int& dst_pos,
                                        const LevelScript* address);
extern void append_jump_if_equal_to_address(LevelScript* dst, int& dst_pos,
                                            u32 value,
                                            const LevelScript* address);

extern void append_execute(LevelScript* dst, int& dst_pos,
                           u8 segment,
                           const u8* segment_start,
                           const u8* segment_end,
                           const LevelScript* address);
extern void append_exit_and_execute(LevelScript* dst, int& dst_pos,
                                    u8 segment,
                                    const u8* segment_start,
                                    const u8* segment_end,
                                    const LevelScript* address);