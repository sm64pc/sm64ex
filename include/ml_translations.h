#ifndef ML_TRANSLATION_H
#define ML_TRANSLATION_H

#include "types.h"

// PAL/multilang support changes most text to arrays for each language. This define allows these
// differences to be combined.
#if defined(VERSION_ML) || defined (VERSION_EU)
    #define LANGUAGE_ARRAY(cmd) cmd[LANGUAGE_FUNCTION]
#else
    #define LANGUAGE_ARRAY(cmd) cmd
#endif

// EU translations are contained in three segment 0x19 compressed data blocks
extern u8 _translation_us_en_mio0SegmentRomStart[];
extern u8 _translation_us_en_mio0SegmentRomEnd[];
extern u8 _translation_uk_en_mio0SegmentRomStart[];
extern u8 _translation_uk_en_mio0SegmentRomEnd[];
extern u8 _translation_jp_mio0SegmentRomStart[];
extern u8 _translation_jp_mio0SegmentRomEnd[];
extern u8 _translation_fr_mio0SegmentRomStart[];
extern u8 _translation_fr_mio0SegmentRomEnd[];
extern u8 _translation_de_mio0SegmentRomStart[];
extern u8 _translation_de_mio0SegmentRomEnd[];

extern void *dialog_table_us_en[];
extern void *course_name_table_us_en[];
extern void *act_name_table_us_en[];

extern void *dialog_table_eu_en[];
extern void *course_name_table_eu_en[];
extern void *act_name_table_eu_en[];

extern void *dialog_table_jp[];
extern void *course_name_table_jp[];
extern void *act_name_table_jp[];

extern void *dialog_table_eu_fr[];
extern void *course_name_table_eu_fr[];
extern void *act_name_table_eu_fr[];

extern void *dialog_table_eu_de[];
extern void *course_name_table_eu_de[];
extern void *act_name_table_eu_de[];

#endif /* ML_TRANSLATION_H */
