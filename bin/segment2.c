#include <ultra64.h>
#include "sm64.h"
#include "game/ingame_menu.h"

#include "make_const_nonconst.h"

// HUD Font
ALIGNED8 static const u8 texture_hud_char_0                          [] = "textures/segment2/segment2.00000.rgba16";
ALIGNED8 static const u8 texture_hud_char_1                          [] = "textures/segment2/segment2.00200.rgba16";
ALIGNED8 static const u8 texture_hud_char_2                          [] = "textures/segment2/segment2.00400.rgba16";
ALIGNED8 static const u8 texture_hud_char_3                          [] = "textures/segment2/segment2.00600.rgba16";
ALIGNED8 static const u8 texture_hud_char_4                          [] = "textures/segment2/segment2.00800.rgba16";
ALIGNED8 static const u8 texture_hud_char_5                          [] = "textures/segment2/segment2.00A00.rgba16";
ALIGNED8 static const u8 texture_hud_char_6                          [] = "textures/segment2/segment2.00C00.rgba16";
ALIGNED8 static const u8 texture_hud_char_7                          [] = "textures/segment2/segment2.00E00.rgba16";
ALIGNED8 static const u8 texture_hud_char_8                          [] = "textures/segment2/segment2.01000.rgba16";
ALIGNED8 static const u8 texture_hud_char_9                          [] = "textures/segment2/segment2.01200.rgba16";
ALIGNED8 static const u8 texture_hud_char_A                          [] = "textures/segment2/segment2.01400.rgba16";
ALIGNED8 static const u8 texture_hud_char_B                          [] = "textures/segment2/segment2.01600.rgba16";
ALIGNED8 static const u8 texture_hud_char_C                          [] = "textures/segment2/segment2.01800.rgba16";
ALIGNED8 static const u8 texture_hud_char_D                          [] = "textures/segment2/segment2.01A00.rgba16";
ALIGNED8 static const u8 texture_hud_char_E                          [] = "textures/segment2/segment2.01C00.rgba16";
ALIGNED8 static const u8 texture_hud_char_F                          [] = "textures/segment2/segment2.01E00.rgba16";
ALIGNED8 static const u8 texture_hud_char_G                          [] = "textures/segment2/segment2.02000.rgba16";
ALIGNED8 static const u8 texture_hud_char_H                          [] = "textures/segment2/segment2.02200.rgba16";
ALIGNED8 static const u8 texture_hud_char_I                          [] = "textures/segment2/segment2.02400.rgba16";
ALIGNED8 static const u8 texture_hud_char_K                          [] = "textures/segment2/segment2.02800.rgba16";
ALIGNED8 static const u8 texture_hud_char_L                          [] = "textures/segment2/segment2.02A00.rgba16";
ALIGNED8 static const u8 texture_hud_char_M                          [] = "textures/segment2/segment2.02C00.rgba16";
ALIGNED8 static const u8 texture_hud_char_N                          [] = "textures/segment2/segment2.02E00.rgba16";
ALIGNED8 static const u8 texture_hud_char_O                          [] = "textures/segment2/segment2.03000.rgba16";
ALIGNED8 static const u8 texture_hud_char_P                          [] = "textures/segment2/segment2.03200.rgba16";
ALIGNED8 static const u8 texture_hud_char_R                          [] = "textures/segment2/segment2.03600.rgba16";
ALIGNED8 static const u8 texture_hud_char_S                          [] = "textures/segment2/segment2.03800.rgba16";
ALIGNED8 static const u8 texture_hud_char_T                          [] = "textures/segment2/segment2.03A00.rgba16";
ALIGNED8 static const u8 texture_hud_char_U                          [] = "textures/segment2/segment2.03C00.rgba16";
ALIGNED8 static const u8 texture_hud_char_W                          [] = "textures/segment2/segment2.04000.rgba16";
ALIGNED8 static const u8 texture_hud_char_Y                          [] = "textures/segment2/segment2.04400.rgba16";
ALIGNED8 static const u8 texture_hud_char_apostrophe                 [] = "textures/segment2/segment2.04800.rgba16";
ALIGNED8 static const u8 texture_hud_char_double_quote               [] = "textures/segment2/segment2.04A00.rgba16";
ALIGNED8 static const u8 texture_hud_char_multiply                   [] = "textures/segment2/segment2.05600.rgba16";
ALIGNED8 static const u8 texture_hud_char_coin                       [] = "textures/segment2/segment2.05800.rgba16";
ALIGNED8 static const u8 texture_hud_char_mario_head                 [] = "textures/segment2/segment2.05A00.rgba16";
ALIGNED8 static const u8 texture_hud_char_star                       [] = "textures/segment2/segment2.05C00.rgba16";
ALIGNED8 static const u8 texture_hud_char_camera                     [] = "textures/segment2/segment2.07B50.rgba16";
ALIGNED8 static const u8 texture_hud_char_lakitu                     [] = "textures/segment2/segment2.07D50.rgba16";
ALIGNED8 static const u8 texture_hud_char_no_camera                  [] = "textures/segment2/segment2.07F50.rgba16";
ALIGNED8 static const u8 texture_hud_char_arrow_up                   [] = "textures/segment2/segment2.08150.rgba16";
ALIGNED8 static const u8 texture_hud_char_arrow_down                 [] = "textures/segment2/segment2.081D0.rgba16";

// HUD Font - Custom Characters
ALIGNED8 static const u8 texture_hud_char_J                          [] = "textures/special/hud_j.rgba16";
ALIGNED8 static const u8 texture_hud_char_Q                          [] = "textures/special/hud_q.rgba16";
ALIGNED8 static const u8 texture_hud_char_V                          [] = "textures/special/hud_v.rgba16";
ALIGNED8 static const u8 texture_hud_char_X                          [] = "textures/special/hud_x.rgba16";
ALIGNED8 static const u8 texture_hud_char_Z                          [] = "textures/special/hud_z.rgba16";
ALIGNED8 static const u8 texture_hud_char_decimal_point              [] = "textures/special/hud_decimal.rgba16";

// Credits Font
ALIGNED8 static const u8 texture_credits_char_3                      [] = "textures/segment2/segment2.06200.rgba16";
ALIGNED8 static const u8 texture_credits_char_4                      [] = "textures/segment2/segment2.06280.rgba16";
ALIGNED8 static const u8 texture_credits_char_6                      [] = "textures/segment2/segment2.06300.rgba16";
ALIGNED8 static const u8 texture_credits_char_A                      [] = "textures/segment2/segment2.06380.rgba16";
ALIGNED8 static const u8 texture_credits_char_B                      [] = "textures/segment2/segment2.06400.rgba16";
ALIGNED8 static const u8 texture_credits_char_C                      [] = "textures/segment2/segment2.06480.rgba16";
ALIGNED8 static const u8 texture_credits_char_D                      [] = "textures/segment2/segment2.06500.rgba16";
ALIGNED8 static const u8 texture_credits_char_E                      [] = "textures/segment2/segment2.06580.rgba16";
ALIGNED8 static const u8 texture_credits_char_F                      [] = "textures/segment2/segment2.06600.rgba16";
ALIGNED8 static const u8 texture_credits_char_G                      [] = "textures/segment2/segment2.06680.rgba16";
ALIGNED8 static const u8 texture_credits_char_H                      [] = "textures/segment2/segment2.06700.rgba16";
ALIGNED8 static const u8 texture_credits_char_I                      [] = "textures/segment2/segment2.06780.rgba16";
ALIGNED8 static const u8 texture_credits_char_J                      [] = "textures/segment2/segment2.06800.rgba16";
ALIGNED8 static const u8 texture_credits_char_K                      [] = "textures/segment2/segment2.06880.rgba16";
ALIGNED8 static const u8 texture_credits_char_L                      [] = "textures/segment2/segment2.06900.rgba16";
ALIGNED8 static const u8 texture_credits_char_M                      [] = "textures/segment2/segment2.06980.rgba16";
ALIGNED8 static const u8 texture_credits_char_N                      [] = "textures/segment2/segment2.06A00.rgba16";
ALIGNED8 static const u8 texture_credits_char_O                      [] = "textures/segment2/segment2.06A80.rgba16";
ALIGNED8 static const u8 texture_credits_char_P                      [] = "textures/segment2/segment2.06B00.rgba16";
ALIGNED8 static const u8 texture_credits_char_Q                      [] = "textures/segment2/segment2.06B80.rgba16";
ALIGNED8 static const u8 texture_credits_char_R                      [] = "textures/segment2/segment2.06C00.rgba16";
ALIGNED8 static const u8 texture_credits_char_S                      [] = "textures/segment2/segment2.06C80.rgba16";
ALIGNED8 static const u8 texture_credits_char_T                      [] = "textures/segment2/segment2.06D00.rgba16";
ALIGNED8 static const u8 texture_credits_char_U                      [] = "textures/segment2/segment2.06D80.rgba16";
ALIGNED8 static const u8 texture_credits_char_V                      [] = "textures/segment2/segment2.06E00.rgba16";
ALIGNED8 static const u8 texture_credits_char_W                      [] = "textures/segment2/segment2.06E80.rgba16";
ALIGNED8 static const u8 texture_credits_char_X                      [] = "textures/segment2/segment2.06F00.rgba16";
ALIGNED8 static const u8 texture_credits_char_Y                      [] = "textures/segment2/segment2.06F80.rgba16";
ALIGNED8 static const u8 texture_credits_char_Z                      [] = "textures/segment2/segment2.07000.rgba16";
ALIGNED8 static const u8 texture_credits_char_period                 [] = "textures/segment2/segment2.07080.rgba16";

// Small Font - US ROM
ALIGNED8 static const u8 texture_font_char_us_0                      [] = "textures/segment2/font_graphics.05900.ia4";
ALIGNED8 static const u8 texture_font_char_us_1                      [] = "textures/segment2/font_graphics.05940.ia4";
ALIGNED8 static const u8 texture_font_char_us_2                      [] = "textures/segment2/font_graphics.05980.ia4";
ALIGNED8 static const u8 texture_font_char_us_3                      [] = "textures/segment2/font_graphics.059C0.ia4";
ALIGNED8 static const u8 texture_font_char_us_4                      [] = "textures/segment2/font_graphics.05A00.ia4";
ALIGNED8 static const u8 texture_font_char_us_5                      [] = "textures/segment2/font_graphics.05A40.ia4";
ALIGNED8 static const u8 texture_font_char_us_6                      [] = "textures/segment2/font_graphics.05A80.ia4";
ALIGNED8 static const u8 texture_font_char_us_7                      [] = "textures/segment2/font_graphics.05AC0.ia4";
ALIGNED8 static const u8 texture_font_char_us_8                      [] = "textures/segment2/font_graphics.05B00.ia4";
ALIGNED8 static const u8 texture_font_char_us_9                      [] = "textures/segment2/font_graphics.05B40.ia4";
ALIGNED8 static const u8 texture_font_char_us_A                      [] = "textures/segment2/font_graphics.05B80.ia4";
ALIGNED8 static const u8 texture_font_char_us_B                      [] = "textures/segment2/font_graphics.05BC0.ia4";
ALIGNED8 static const u8 texture_font_char_us_C                      [] = "textures/segment2/font_graphics.05C00.ia4";
ALIGNED8 static const u8 texture_font_char_us_D                      [] = "textures/segment2/font_graphics.05C40.ia4";
ALIGNED8 static const u8 texture_font_char_us_E                      [] = "textures/segment2/font_graphics.05C80.ia4";
ALIGNED8 static const u8 texture_font_char_us_F                      [] = "textures/segment2/font_graphics.05CC0.ia4";
ALIGNED8 static const u8 texture_font_char_us_G                      [] = "textures/segment2/font_graphics.05D00.ia4";
ALIGNED8 static const u8 texture_font_char_us_H                      [] = "textures/segment2/font_graphics.05D40.ia4";
ALIGNED8 static const u8 texture_font_char_us_I                      [] = "textures/segment2/font_graphics.05D80.ia4";
ALIGNED8 static const u8 texture_font_char_us_J                      [] = "textures/segment2/font_graphics.05DC0.ia4";
ALIGNED8 static const u8 texture_font_char_us_K                      [] = "textures/segment2/font_graphics.05E00.ia4";
ALIGNED8 static const u8 texture_font_char_us_L                      [] = "textures/segment2/font_graphics.05E40.ia4";
ALIGNED8 static const u8 texture_font_char_us_M                      [] = "textures/segment2/font_graphics.05E80.ia4";
ALIGNED8 static const u8 texture_font_char_us_N                      [] = "textures/segment2/font_graphics.05EC0.ia4";
ALIGNED8 static const u8 texture_font_char_us_O                      [] = "textures/segment2/font_graphics.05F00.ia4";
ALIGNED8 static const u8 texture_font_char_us_P                      [] = "textures/segment2/font_graphics.05F40.ia4";
ALIGNED8 static const u8 texture_font_char_us_Q                      [] = "textures/segment2/font_graphics.05F80.ia4";
ALIGNED8 static const u8 texture_font_char_us_R                      [] = "textures/segment2/font_graphics.05FC0.ia4";
ALIGNED8 static const u8 texture_font_char_us_S                      [] = "textures/segment2/font_graphics.06000.ia4";
ALIGNED8 static const u8 texture_font_char_us_T                      [] = "textures/segment2/font_graphics.06040.ia4";
ALIGNED8 static const u8 texture_font_char_us_U                      [] = "textures/segment2/font_graphics.06080.ia4";
ALIGNED8 static const u8 texture_font_char_us_V                      [] = "textures/segment2/font_graphics.060C0.ia4";
ALIGNED8 static const u8 texture_font_char_us_W                      [] = "textures/segment2/font_graphics.06100.ia4";
ALIGNED8 static const u8 texture_font_char_us_X                      [] = "textures/segment2/font_graphics.06140.ia4";
ALIGNED8 static const u8 texture_font_char_us_Y                      [] = "textures/segment2/font_graphics.06180.ia4";
ALIGNED8 static const u8 texture_font_char_us_Z                      [] = "textures/segment2/font_graphics.061C0.ia4";
ALIGNED8 static const u8 texture_font_char_us_a                      [] = "textures/segment2/font_graphics.06200.ia4";
ALIGNED8 static const u8 texture_font_char_us_b                      [] = "textures/segment2/font_graphics.06240.ia4";
ALIGNED8 static const u8 texture_font_char_us_c                      [] = "textures/segment2/font_graphics.06280.ia4";
ALIGNED8 static const u8 texture_font_char_us_d                      [] = "textures/segment2/font_graphics.062C0.ia4";
ALIGNED8 static const u8 texture_font_char_us_e                      [] = "textures/segment2/font_graphics.06300.ia4";
ALIGNED8 static const u8 texture_font_char_us_f                      [] = "textures/segment2/font_graphics.06340.ia4";
ALIGNED8 static const u8 texture_font_char_us_g                      [] = "textures/segment2/font_graphics.06380.ia4";
ALIGNED8 static const u8 texture_font_char_us_h                      [] = "textures/segment2/font_graphics.063C0.ia4";
ALIGNED8 static const u8 texture_font_char_us_i                      [] = "textures/segment2/font_graphics.06400.ia4";
ALIGNED8 static const u8 texture_font_char_us_j                      [] = "textures/segment2/font_graphics.06440.ia4";
ALIGNED8 static const u8 texture_font_char_us_k                      [] = "textures/segment2/font_graphics.06480.ia4";
ALIGNED8 static const u8 texture_font_char_us_l                      [] = "textures/segment2/font_graphics.064C0.ia4";
ALIGNED8 static const u8 texture_font_char_us_m                      [] = "textures/segment2/font_graphics.06500.ia4";
ALIGNED8 static const u8 texture_font_char_us_n                      [] = "textures/segment2/font_graphics.06540.ia4";
ALIGNED8 static const u8 texture_font_char_us_o                      [] = "textures/segment2/font_graphics.06580.ia4";
ALIGNED8 static const u8 texture_font_char_us_p                      [] = "textures/segment2/font_graphics.065C0.ia4";
ALIGNED8 static const u8 texture_font_char_us_q                      [] = "textures/segment2/font_graphics.06600.ia4";
ALIGNED8 static const u8 texture_font_char_us_r                      [] = "textures/segment2/font_graphics.06640.ia4";
ALIGNED8 static const u8 texture_font_char_us_s                      [] = "textures/segment2/font_graphics.06680.ia4";
ALIGNED8 static const u8 texture_font_char_us_t                      [] = "textures/segment2/font_graphics.066C0.ia4";
ALIGNED8 static const u8 texture_font_char_us_u                      [] = "textures/segment2/font_graphics.06700.ia4";
ALIGNED8 static const u8 texture_font_char_us_v                      [] = "textures/segment2/font_graphics.06740.ia4";
ALIGNED8 static const u8 texture_font_char_us_w                      [] = "textures/segment2/font_graphics.06780.ia4";
ALIGNED8 static const u8 texture_font_char_us_x                      [] = "textures/segment2/font_graphics.067C0.ia4";
ALIGNED8 static const u8 texture_font_char_us_y                      [] = "textures/segment2/font_graphics.06800.ia4";
ALIGNED8 static const u8 texture_font_char_us_z                      [] = "textures/segment2/font_graphics.06840.ia4";
ALIGNED8 static const u8 texture_font_char_us_left_right_arrow       [] = "textures/segment2/font_graphics.06880.ia4";
ALIGNED8 static const u8 texture_font_char_us_exclamation            [] = "textures/segment2/font_graphics.068C0.ia4";
ALIGNED8 static const u8 texture_font_char_us_coin                   [] = "textures/segment2/font_graphics.06900.ia4";
ALIGNED8 static const u8 texture_font_char_us_multiply               [] = "textures/segment2/font_graphics.06940.ia4";
ALIGNED8 static const u8 texture_font_char_us_open_parentheses       [] = "textures/segment2/font_graphics.06980.ia4";
ALIGNED8 static const u8 texture_font_char_us_close_open_parentheses [] = "textures/segment2/font_graphics.069C0.ia4";
ALIGNED8 static const u8 texture_font_char_us_close_parentheses      [] = "textures/segment2/font_graphics.06A00.ia4";
ALIGNED8 static const u8 texture_font_char_us_tilde                  [] = "textures/segment2/font_graphics.06A40.ia4";
ALIGNED8 static const u8 texture_font_char_us_period                 [] = "textures/segment2/font_graphics.06A80.ia4";
ALIGNED8 static const u8 texture_font_char_us_percent                [] = "textures/segment2/font_graphics.06AC0.ia4";
ALIGNED8 static const u8 texture_font_char_us_interpunct             [] = "textures/segment2/font_graphics.06B00.ia4";
ALIGNED8 static const u8 texture_font_char_us_comma                  [] = "textures/segment2/font_graphics.06B40.ia4";
ALIGNED8 static const u8 texture_font_char_us_apostrophe             [] = "textures/segment2/font_graphics.06B80.ia4";
ALIGNED8 static const u8 texture_font_char_us_question               [] = "textures/segment2/font_graphics.06BC0.ia4";
ALIGNED8 static const u8 texture_font_char_us_star_filled            [] = "textures/segment2/font_graphics.06C00.ia4";
ALIGNED8 static const u8 texture_font_char_us_star_hollow            [] = "textures/segment2/font_graphics.06C40.ia4";
ALIGNED8 static const u8 texture_font_char_us_double_quote_open      [] = "textures/segment2/font_graphics.06C80.ia4";
ALIGNED8 static const u8 texture_font_char_us_double_quote_close     [] = "textures/segment2/font_graphics.06CC0.ia4";
ALIGNED8 static const u8 texture_font_char_us_ellipsis               [] = "textures/segment2/font_graphics.06D00.ia4";
ALIGNED8 static const u8 texture_font_char_us_slash                  [] = "textures/segment2/font_graphics.06D40.ia4";
ALIGNED8 static const u8 texture_font_char_us_ampersand              [] = "textures/segment2/font_graphics.06D80.ia4";
ALIGNED8 static const u8 texture_font_char_us_button_A               [] = "textures/segment2/font_graphics.06DC0.ia4";
ALIGNED8 static const u8 texture_font_char_us_button_B               [] = "textures/segment2/font_graphics.06E00.ia4";
ALIGNED8 static const u8 texture_font_char_us_button_C               [] = "textures/segment2/font_graphics.06E40.ia4";
ALIGNED8 static const u8 texture_font_char_us_button_Z               [] = "textures/segment2/font_graphics.06E80.ia4";
ALIGNED8 static const u8 texture_font_char_us_button_R               [] = "textures/segment2/font_graphics.06EC0.ia4";
ALIGNED8 static const u8 texture_font_char_us_button_C_up            [] = "textures/segment2/font_graphics.06F00.ia4";
ALIGNED8 static const u8 texture_font_char_us_button_C_down          [] = "textures/segment2/font_graphics.06F40.ia4";
ALIGNED8 static const u8 texture_font_char_us_button_C_left          [] = "textures/segment2/font_graphics.06F80.ia4";
ALIGNED8 static const u8 texture_font_char_us_button_C_right         [] = "textures/segment2/font_graphics.06FC0.ia4";

// Small Font - Custom Characters
ALIGNED8 static const u8 texture_font_char_cust_inverted_exclamation [] = "textures/special/character_00A1.ia4";
//
ALIGNED8 static const u8 texture_font_char_cust_inverted_question    [] = "textures/special/character_00BF.ia4";
ALIGNED8 static const u8 texture_font_char_cust_A_grave              [] = "textures/special/character_00C0.ia4";
ALIGNED8 static const u8 texture_font_char_cust_A_acute              [] = "textures/special/character_00C1.ia4";
ALIGNED8 static const u8 texture_font_char_cust_A_circumflex         [] = "textures/special/character_00C2.ia4";
ALIGNED8 static const u8 texture_font_char_cust_A_tilde              [] = "textures/special/character_00C3.ia4";
ALIGNED8 static const u8 texture_font_char_cust_A_diaeresis          [] = "textures/special/character_00C4.ia4";
ALIGNED8 static const u8 texture_font_char_cust_A_ring               [] = "textures/special/character_00C5.ia4";
ALIGNED8 static const u8 texture_font_char_cust_AE                   [] = "textures/special/character_00C6.ia4";
ALIGNED8 static const u8 texture_font_char_cust_C_cedilla            [] = "textures/special/character_00C7.ia4";
ALIGNED8 static const u8 texture_font_char_cust_E_grave              [] = "textures/special/character_00C8.ia4";
ALIGNED8 static const u8 texture_font_char_cust_E_acute              [] = "textures/special/character_00C9.ia4";
ALIGNED8 static const u8 texture_font_char_cust_E_circumflex         [] = "textures/special/character_00CA.ia4";
ALIGNED8 static const u8 texture_font_char_cust_E_diaeresis          [] = "textures/special/character_00CB.ia4";
ALIGNED8 static const u8 texture_font_char_cust_I_grave              [] = "textures/special/character_00CC.ia4";
ALIGNED8 static const u8 texture_font_char_cust_I_acute              [] = "textures/special/character_00CD.ia4";
ALIGNED8 static const u8 texture_font_char_cust_I_circumflex         [] = "textures/special/character_00CE.ia4";
ALIGNED8 static const u8 texture_font_char_cust_I_diaeresis          [] = "textures/special/character_00CF.ia4";
ALIGNED8 static const u8 texture_font_char_cust_ETH                  [] = "textures/special/character_00D0.ia4";
ALIGNED8 static const u8 texture_font_char_cust_N_tilde              [] = "textures/special/character_00D1.ia4";
ALIGNED8 static const u8 texture_font_char_cust_O_grave              [] = "textures/special/character_00D2.ia4";
ALIGNED8 static const u8 texture_font_char_cust_O_acute              [] = "textures/special/character_00D3.ia4";
ALIGNED8 static const u8 texture_font_char_cust_O_circumflex         [] = "textures/special/character_00D4.ia4";
ALIGNED8 static const u8 texture_font_char_cust_O_tilde              [] = "textures/special/character_00D5.ia4";
ALIGNED8 static const u8 texture_font_char_cust_O_diaeresis          [] = "textures/special/character_00D6.ia4";
//
ALIGNED8 static const u8 texture_font_char_cust_O_stroke             [] = "textures/special/character_00D8.ia4";
ALIGNED8 static const u8 texture_font_char_cust_U_grave              [] = "textures/special/character_00D9.ia4";
ALIGNED8 static const u8 texture_font_char_cust_U_acute              [] = "textures/special/character_00DA.ia4";
ALIGNED8 static const u8 texture_font_char_cust_U_circumflex         [] = "textures/special/character_00DB.ia4";
ALIGNED8 static const u8 texture_font_char_cust_U_diaeresis          [] = "textures/special/character_00DC.ia4";
ALIGNED8 static const u8 texture_font_char_cust_Y_acute              [] = "textures/special/character_00DD.ia4";
ALIGNED8 static const u8 texture_font_char_cust_THORN                [] = "textures/special/character_00DE.ia4";
ALIGNED8 static const u8 texture_font_char_cust_ESZETT               [] = "textures/special/character_00DF.ia4";
ALIGNED8 static const u8 texture_font_char_cust_a_grave              [] = "textures/special/character_00E0.ia4";
ALIGNED8 static const u8 texture_font_char_cust_a_acute              [] = "textures/special/character_00E1.ia4";
ALIGNED8 static const u8 texture_font_char_cust_a_circumflex         [] = "textures/special/character_00E2.ia4";
ALIGNED8 static const u8 texture_font_char_cust_a_tilde              [] = "textures/special/character_00E3.ia4";
ALIGNED8 static const u8 texture_font_char_cust_a_diaeresis          [] = "textures/special/character_00E4.ia4";
ALIGNED8 static const u8 texture_font_char_cust_a_ring               [] = "textures/special/character_00E5.ia4";
ALIGNED8 static const u8 texture_font_char_cust_ae                   [] = "textures/special/character_00E6.ia4";
ALIGNED8 static const u8 texture_font_char_cust_c_cedilla            [] = "textures/special/character_00E7.ia4";
ALIGNED8 static const u8 texture_font_char_cust_e_grave              [] = "textures/special/character_00E8.ia4";
ALIGNED8 static const u8 texture_font_char_cust_e_acute              [] = "textures/special/character_00E9.ia4";
ALIGNED8 static const u8 texture_font_char_cust_e_circumflex         [] = "textures/special/character_00EA.ia4";
ALIGNED8 static const u8 texture_font_char_cust_e_diaeresis          [] = "textures/special/character_00EB.ia4";
ALIGNED8 static const u8 texture_font_char_cust_i_grave              [] = "textures/special/character_00EC.ia4";
ALIGNED8 static const u8 texture_font_char_cust_i_acute              [] = "textures/special/character_00ED.ia4";
ALIGNED8 static const u8 texture_font_char_cust_i_circumflex         [] = "textures/special/character_00EE.ia4";
ALIGNED8 static const u8 texture_font_char_cust_i_diaeresis          [] = "textures/special/character_00EF.ia4";
ALIGNED8 static const u8 texture_font_char_cust_eth                  [] = "textures/special/character_00F0.ia4";
ALIGNED8 static const u8 texture_font_char_cust_n_tilde              [] = "textures/special/character_00F1.ia4";
ALIGNED8 static const u8 texture_font_char_cust_o_grave              [] = "textures/special/character_00F2.ia4";
ALIGNED8 static const u8 texture_font_char_cust_o_acute              [] = "textures/special/character_00F3.ia4";
ALIGNED8 static const u8 texture_font_char_cust_o_circumflex         [] = "textures/special/character_00F4.ia4";
ALIGNED8 static const u8 texture_font_char_cust_o_tilde              [] = "textures/special/character_00F5.ia4";
ALIGNED8 static const u8 texture_font_char_cust_o_diaeresis          [] = "textures/special/character_00F6.ia4";
//
ALIGNED8 static const u8 texture_font_char_cust_o_stroke             [] = "textures/special/character_00F8.ia4";
ALIGNED8 static const u8 texture_font_char_cust_u_grave              [] = "textures/special/character_00F9.ia4";
ALIGNED8 static const u8 texture_font_char_cust_u_acute              [] = "textures/special/character_00FA.ia4";
ALIGNED8 static const u8 texture_font_char_cust_u_circumflex         [] = "textures/special/character_00FB.ia4";
ALIGNED8 static const u8 texture_font_char_cust_u_diaeresis          [] = "textures/special/character_00FC.ia4";
ALIGNED8 static const u8 texture_font_char_cust_y_acute              [] = "textures/special/character_00FD.ia4";
ALIGNED8 static const u8 texture_font_char_cust_thorn                [] = "textures/special/character_00FE.ia4";
ALIGNED8 static const u8 texture_font_char_cust_y_diaeresis          [] = "textures/special/character_00FF.ia4";
//
ALIGNED8 static const u8 texture_font_char_cust_A_ogonek             [] = "textures/special/character_0104.ia4";
ALIGNED8 static const u8 texture_font_char_cust_a_ogonek             [] = "textures/special/character_0105.ia4";
ALIGNED8 static const u8 texture_font_char_cust_C_acute              [] = "textures/special/character_0106.ia4";
ALIGNED8 static const u8 texture_font_char_cust_c_acute              [] = "textures/special/character_0107.ia4";
//
ALIGNED8 static const u8 texture_font_char_cust_E_ogonek             [] = "textures/special/character_0118.ia4";
ALIGNED8 static const u8 texture_font_char_cust_e_ogonek             [] = "textures/special/character_0119.ia4";
//
ALIGNED8 static const u8 texture_font_char_cust_G_breve              [] = "textures/special/character_011E.ia4";
ALIGNED8 static const u8 texture_font_char_cust_g_breve              [] = "textures/special/character_011F.ia4";
//
ALIGNED8 static const u8 texture_font_char_cust_S_cedilla            [] = "textures/special/character_0130.ia4";
ALIGNED8 static const u8 texture_font_char_cust_s_cedilla            [] = "textures/special/character_0131.ia4";
//
ALIGNED8 static const u8 texture_font_char_cust_L_stroke             [] = "textures/special/character_0141.ia4";
ALIGNED8 static const u8 texture_font_char_cust_l_stroke             [] = "textures/special/character_0142.ia4";
ALIGNED8 static const u8 texture_font_char_cust_N_acute              [] = "textures/special/character_0143.ia4";
ALIGNED8 static const u8 texture_font_char_cust_n_acute              [] = "textures/special/character_0144.ia4";
//
ALIGNED8 static const u8 texture_font_char_cust_OE                   [] = "textures/special/character_0152.ia4";
ALIGNED8 static const u8 texture_font_char_cust_oe                   [] = "textures/special/character_0153.ia4";
//
ALIGNED8 static const u8 texture_font_char_cust_S_acute              [] = "textures/special/character_015A.ia4";
ALIGNED8 static const u8 texture_font_char_cust_s_acute              [] = "textures/special/character_015B.ia4";
//
ALIGNED8 static const u8 texture_font_char_cust_i_dot                [] = "textures/special/character_015E.ia4";
ALIGNED8 static const u8 texture_font_char_cust_i_dotless            [] = "textures/special/character_015F.ia4";
//
ALIGNED8 static const u8 texture_font_char_cust_Y_diaeresis          [] = "textures/special/character_0178.ia4";
ALIGNED8 static const u8 texture_font_char_cust_Z_acute              [] = "textures/special/character_0179.ia4";
ALIGNED8 static const u8 texture_font_char_cust_z_acute              [] = "textures/special/character_017A.ia4";
ALIGNED8 static const u8 texture_font_char_cust_Z_dot                [] = "textures/special/character_017B.ia4";
ALIGNED8 static const u8 texture_font_char_cust_z_dot                [] = "textures/special/character_017C.ia4";
//
ALIGNED8 static const u8 texture_font_char_cust_eszett               [] = "textures/special/character_1E9E.ia4";
ALIGNED8 static const u8 texture_font_char_cust_plus                 [] = "textures/special/character_002B.ia4";

ALIGNED8 static const u8 texture_menu_font_char_mface1               [] = "textures/special/character_263A.left.ia4";
ALIGNED8 static const u8 texture_menu_font_char_mface2               [] = "textures/special/character_263A.right.ia4";

// Main HUD print table 0x02008250-0x02008337
const u8 *const main_hud_lut[] = {
    texture_hud_char_0, texture_hud_char_1, texture_hud_char_2, texture_hud_char_3,
    texture_hud_char_4, texture_hud_char_5, texture_hud_char_6, texture_hud_char_7,
    texture_hud_char_8, texture_hud_char_9, texture_hud_char_A, texture_hud_char_B,
    texture_hud_char_C, texture_hud_char_D, texture_hud_char_E, texture_hud_char_F,
    texture_hud_char_G, texture_hud_char_H, texture_hud_char_I, texture_hud_char_J,
    texture_hud_char_K, texture_hud_char_L, texture_hud_char_M, texture_hud_char_N,
    texture_hud_char_O, texture_hud_char_P, texture_hud_char_Q, texture_hud_char_R,
    texture_hud_char_S, texture_hud_char_T, texture_hud_char_U, texture_hud_char_V,
    texture_hud_char_W, texture_hud_char_X, texture_hud_char_Y, texture_hud_char_Z,
    0x0,                 0x0,                      0x0,                  0x0,
    0x0,                 0x0,                      0x0,                  0x0,
    0x0,                 0x0,                      0x0,                  0x0,
    0x0,                 0x0, texture_hud_char_multiply, texture_hud_char_coin,
    texture_hud_char_mario_head, texture_hud_char_star, texture_hud_char_decimal_point, 0x0,
    texture_hud_char_apostrophe, texture_hud_char_double_quote,
};

// Main small font print table 0x02008338-0x02008737
const u8 *const main_font_lut[] = {
    texture_font_char_us_0                                   , texture_font_char_us_1                                   ,
    texture_font_char_us_2                                   , texture_font_char_us_3                                   ,
    texture_font_char_us_4                                   , texture_font_char_us_5                                   ,
    texture_font_char_us_6                                   , texture_font_char_us_7                                   ,
    texture_font_char_us_8                                   , texture_font_char_us_9                                   ,
    texture_font_char_us_A                                   , texture_font_char_us_B                                   ,
    texture_font_char_us_C                                   , texture_font_char_us_D                                   ,
    texture_font_char_us_E                                   , texture_font_char_us_F                                   ,
    texture_font_char_us_G                                   , texture_font_char_us_H                                   ,
    texture_font_char_us_I                                   , texture_font_char_us_J                                   ,
    texture_font_char_us_K                                   , texture_font_char_us_L                                   ,
    texture_font_char_us_M                                   , texture_font_char_us_N                                   ,
    texture_font_char_us_O                                   , texture_font_char_us_P                                   ,
    texture_font_char_us_Q                                   , texture_font_char_us_R                                   ,
    texture_font_char_us_S                                   , texture_font_char_us_T                                   ,
    texture_font_char_us_U                                   , texture_font_char_us_V                                   ,
    texture_font_char_us_W                                   , texture_font_char_us_X                                   ,
    texture_font_char_us_Y                                   , texture_font_char_us_Z                                   ,
    texture_font_char_us_a                                   , texture_font_char_us_b                                   ,
    texture_font_char_us_c                                   , texture_font_char_us_d                                   ,
    texture_font_char_us_e                                   , texture_font_char_us_f                                   ,
    texture_font_char_us_g                                   , texture_font_char_us_h                                   ,
    texture_font_char_us_i                                   , texture_font_char_us_j                                   ,
    texture_font_char_us_k                                   , texture_font_char_us_l                                   ,
    texture_font_char_us_m                                   , texture_font_char_us_n                                   ,
    texture_font_char_us_o                                   , texture_font_char_us_p                                   ,
    texture_font_char_us_q                                   , texture_font_char_us_r                                   ,
    texture_font_char_us_s                                   , texture_font_char_us_t                                   ,
    texture_font_char_us_u                                   , texture_font_char_us_v                                   ,
    texture_font_char_us_w                                   , texture_font_char_us_x                                   ,
    texture_font_char_us_y                                   , texture_font_char_us_z                                   ,
    texture_font_char_us_apostrophe                          , texture_font_char_us_period                              ,
    texture_menu_font_char_mface1     /* <Mario face left> */, texture_menu_font_char_mface2    /* <Mario face right> */,
    texture_font_char_cust_inverted_exclamation              , texture_font_char_cust_inverted_question                 ,
    texture_font_char_cust_A_grave                           , texture_font_char_cust_A_acute                           ,
    texture_font_char_cust_A_circumflex                      , texture_font_char_cust_A_tilde                           ,
    texture_font_char_cust_A_diaeresis                       , texture_font_char_cust_A_ring                            ,
    texture_font_char_cust_AE                                , texture_font_char_cust_C_cedilla                         ,
    texture_font_char_cust_E_grave                           , texture_font_char_cust_E_acute                           ,
    texture_font_char_cust_E_circumflex                      , texture_font_char_cust_E_diaeresis                       ,
    texture_font_char_us_button_C_up                /* '^' */, texture_font_char_us_button_C_down              /* '|' */,
    texture_font_char_us_button_C_left              /* '<' */, texture_font_char_us_button_C_right             /* '>' */,
    texture_font_char_us_button_A                 /* '[A]' */, texture_font_char_us_button_B                 /* '[B]' */,
    texture_font_char_us_button_C                 /* '[C]' */, texture_font_char_us_button_Z                 /* '[Z]' */,
    texture_font_char_us_button_R                 /* '[R]' */, texture_font_char_cust_I_grave                           ,
    texture_font_char_cust_I_acute                           , texture_font_char_cust_I_circumflex                      ,
    texture_font_char_cust_I_diaeresis                       , texture_font_char_cust_ETH                               ,
    texture_font_char_cust_N_tilde                           , texture_font_char_cust_O_grave                           ,
    texture_font_char_cust_O_acute                           , texture_font_char_cust_O_circumflex                      ,
    texture_font_char_cust_O_tilde                           , texture_font_char_cust_O_diaeresis                       ,
    texture_font_char_cust_O_stroke                          , texture_font_char_cust_U_grave                           ,
    texture_font_char_cust_U_acute                           , texture_font_char_cust_U_circumflex                      ,
    texture_font_char_cust_U_diaeresis                       , texture_font_char_cust_Y_acute                           ,
    texture_font_char_cust_THORN                             , texture_font_char_cust_ESZETT                            ,
    texture_font_char_cust_a_grave                           , texture_font_char_cust_a_acute                           ,
    texture_font_char_cust_a_circumflex                      , texture_font_char_us_comma                      /* ',' */,
    texture_font_char_cust_a_tilde                           , texture_font_char_cust_a_diaeresis                       ,
    texture_font_char_cust_a_ring                            , texture_font_char_cust_ae                                ,
    texture_font_char_cust_c_cedilla                         , texture_font_char_cust_e_grave                           ,
    texture_font_char_cust_e_acute                           , texture_font_char_cust_e_circumflex                      ,
    texture_font_char_cust_e_diaeresis                       , texture_font_char_cust_i_grave                           ,
    texture_font_char_cust_i_acute                           , texture_font_char_cust_i_circumflex                      ,
    texture_font_char_cust_i_diaeresis                       , texture_font_char_cust_eth                               ,
    texture_font_char_cust_n_tilde                           , texture_font_char_cust_o_grave                           ,
    texture_font_char_cust_o_acute                           , texture_font_char_cust_o_circumflex                      ,
    texture_font_char_cust_o_tilde                           , texture_font_char_cust_o_diaeresis                       ,
    texture_font_char_cust_o_stroke                          , texture_font_char_cust_u_grave                           ,
    texture_font_char_cust_u_acute                           , texture_font_char_cust_u_circumflex                      ,
    texture_font_char_cust_u_diaeresis                       , texture_font_char_cust_y_acute                           ,
    texture_font_char_cust_thorn                             , texture_font_char_cust_y_diaeresis                       ,
    texture_font_char_cust_A_ogonek                          , texture_font_char_cust_a_ogonek                          ,
    texture_font_char_cust_C_acute                           , texture_font_char_cust_c_acute                           ,
    texture_font_char_cust_E_ogonek                          , texture_font_char_cust_e_ogonek                          ,
    texture_font_char_cust_G_breve                           , texture_font_char_cust_g_breve                           ,
    texture_font_char_cust_S_cedilla                         , texture_font_char_cust_s_cedilla                         ,
    texture_font_char_cust_L_stroke                          , texture_font_char_cust_l_stroke                          ,
    texture_font_char_cust_N_acute                           , texture_font_char_cust_n_acute                           ,
    texture_font_char_cust_OE                                , texture_font_char_cust_oe                                ,
    texture_font_char_cust_S_acute                           , texture_font_char_cust_s_acute                           ,
    0x0                                         /* <space> */, texture_font_char_us_slash                      /* '-' */,
    texture_font_char_cust_i_dot                             , texture_font_char_cust_i_dotless                         ,
    texture_font_char_cust_Y_diaeresis                       , texture_font_char_cust_Z_acute                           ,
    texture_font_char_cust_z_acute                           , texture_font_char_cust_Z_dot                             ,
    texture_font_char_cust_z_dot                             , texture_font_char_cust_eszett                            ,
    0x0                                                      , 0x0                                                      ,
    0x0                                                      , 0x0                                                      ,
    0x0                                                      , 0x0                                                      ,
    0x0                                                      , 0x0                                                      ,
    0x0                                                      , 0x0                                                      ,
    0x0                                                      , 0x0                                                      ,
    0x0                                                      , 0x0                                                      ,
    0x0                                                      , 0x0                                                      ,
    0x0                                                      , 0x0                                                      ,
    0x0                                                      , 0x0                                                      ,
    0x0                                                      , 0x0                                                      ,
    0x0                                                      , 0x0                                                      ,
    0x0                                                      , 0x0                                                      ,
    0x0                                                      , 0x0                                                      ,
    0x0                                                      , 0x0                                                      ,
    0x0                                                      , 0x0                                                      ,
    0x0                                                      , 0x0                                                      ,
    0x0                                                      , 0x0                                                      ,
    0x0                                                      , 0x0                                                      ,
    0x0                                                      , 0x0                                                      ,
    0x0                                           /* <tab> */, 0x0                                           /* 'the' */,
    0x0                                           /* 'you' */, 0x0                                                      ,
    0x0                                                      , 0x0                                                      ,
    0x0                                                      , 0x0                                                      ,
    0x0                                                      , 0x0                                                      ,
    0x0                                                      , 0x0                                                      ,
    0x0                                                      , 0x0                                                      ,
    0x0                                                      , 0x0                                                      ,
    0x0                                 /* <digit counter> */, texture_font_char_us_open_parentheses           /* '(' */,
    texture_font_char_us_close_open_parentheses     /* ')' */, texture_font_char_us_close_parentheses         /* ')(' */,
    texture_font_char_us_left_right_arrow         /* '<->' */, texture_font_char_us_ampersand                  /* '&' */,
    texture_font_char_us_ellipsis                   /* ':' */, texture_font_char_cust_plus                     /* '+' */,
    0x0                                                      , 0x0                                                      ,
    0x0                                                      , 0x0                                                      ,
    0x0                                                      , 0x0                                                      ,
    0x0                                                      , 0x0                                                      ,
    0x0                 /* leftover <JP voiced sound mark> */, 0x0            /* leftover <JP semi-voiced sound mark> */,
    texture_font_char_us_exclamation                /* '!' */, texture_font_char_us_percent                    /* '%' */,
    texture_font_char_us_question                   /* '?' */, texture_font_char_us_double_quote_open          /* '"' */,
    texture_font_char_us_double_quote_close         /* '"' */, texture_font_char_us_tilde                      /* '~' */,
    0x0                                      /* <ellipses> */, texture_font_char_us_coin                    /* <coin> */,
    texture_font_char_us_star_filled      /* <filled star> */, texture_font_char_us_multiply /* <multiplication sign> */,
    texture_font_char_us_interpunct        /* <interpunct> */, texture_font_char_us_star_hollow      /* <hollow star> */,
    0x0                                       /* <newline> */, 0x0                                                      ,
};

// credits font LUT 0x02008738-0x020087CB
const u8 *const main_credits_font_lut[] = {
                       0x0,                    0x0,                    0x0, texture_credits_char_3,
    texture_credits_char_4,                    0x0, texture_credits_char_6,                    0x0,
                       0x0,                    0x0, texture_credits_char_A, texture_credits_char_B,
    texture_credits_char_C, texture_credits_char_D, texture_credits_char_E, texture_credits_char_F,
    texture_credits_char_G, texture_credits_char_H, texture_credits_char_I, texture_credits_char_J,
    texture_credits_char_K, texture_credits_char_L, texture_credits_char_M, texture_credits_char_N,
    texture_credits_char_O, texture_credits_char_P, texture_credits_char_Q, texture_credits_char_R,
    texture_credits_char_S, texture_credits_char_T, texture_credits_char_U, texture_credits_char_V,
    texture_credits_char_W, texture_credits_char_X, texture_credits_char_Y, texture_credits_char_Z,
    texture_credits_char_period,
};

// HUD camera table 0x020087CC-0x020087E3
const u8 *const main_hud_camera_lut[] = {
    texture_hud_char_camera, texture_hud_char_mario_head, texture_hud_char_lakitu, texture_hud_char_no_camera,
    texture_hud_char_arrow_up, texture_hud_char_arrow_down,
};

UNUSED static const u64 segment2_unused_0 = 0;

// 0x0200EC60 - 0x0200EC98
const Gfx dl_hud_img_begin[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_COPY),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsDPSetBlendColor(255, 255, 255, 255),
#ifdef VERSION_EU
    gsDPSetRenderMode(G_RM_NOOP, G_RM_NOOP2),
    gsDPSetTextureFilter(G_TF_POINT),
#else
    gsDPSetRenderMode(G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2),
#endif
    gsSPEndDisplayList(),
};

// 0x0200EC98 - 0x0200ECC8
const Gfx dl_hud_img_load_tex_block[] = {
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, 4, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 4, G_TX_NOLOD),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 16 * 16 - 1, CALC_DXT(16, G_IM_SIZ_16b_BYTES)),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 4, 0, G_TX_RENDERTILE, 0, G_TX_CLAMP, 4, G_TX_NOLOD, G_TX_CLAMP, 4, G_TX_NOLOD),
    gsDPSetTileSize(0, 0, 0, (16 - 1) << G_TEXTURE_IMAGE_FRAC, (16 - 1) << G_TEXTURE_IMAGE_FRAC),
    gsSPEndDisplayList(),
};

// 0x0200ECC8 - 0x0200ED00
const Gfx dl_hud_img_end[] = {
    gsDPPipeSync(),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetRenderMode(G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2),
    gsDPSetAlphaCompare(G_AC_NONE),
#ifdef VERSION_EU
    gsDPSetTextureFilter(G_TF_BILERP),
    gsDPSetCycleType(G_CYC_1CYCLE),
#else
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF),
#endif
    gsSPEndDisplayList(),
};

// 0x0200ED00 - 0x0200ED38
const Gfx dl_rgba16_text_begin[] = {
    gsDPPipeSync(),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetCombineMode(G_CC_FADEA, G_CC_FADEA),
    gsDPSetEnvColor(255, 255, 255, 255),
    gsDPSetRenderMode(G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2),
    gsDPSetTextureFilter(G_TF_POINT),
    gsSPEndDisplayList(),
};

// 0x0200ED38 - 0x0200ED68
const Gfx dl_rgba16_load_tex_block[] = {
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, 4, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 4, G_TX_NOLOD),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 16 * 16 - 1, CALC_DXT(16, G_IM_SIZ_16b_BYTES)),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 4, 0, G_TX_RENDERTILE, 0, G_TX_CLAMP, 4, G_TX_NOLOD, G_TX_CLAMP, 4, G_TX_NOLOD),
    gsDPSetTileSize(0, 0, 0, (16 - 1) << G_TEXTURE_IMAGE_FRAC, (16 - 1) << G_TEXTURE_IMAGE_FRAC),
    gsSPEndDisplayList(),
};

// 0x0200ED68 - 0x0200EDA8
const Gfx dl_rgba16_text_end[] = {
    gsDPPipeSync(),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetRenderMode(G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsDPSetEnvColor(255, 255, 255, 255),
    gsDPSetTextureFilter(G_TF_BILERP),
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF),
    gsSPEndDisplayList(),
};

// 0x0200EDA8 - 0x0200EDE8
static const Vtx vertex_text_bg_box[] = {
    {{{     0,    -80,      0}, 0, {     0,      0}, {0xff, 0xff, 0xff, 0xff}}},
    {{{   130,    -80,      0}, 0, {     0,      0}, {0xff, 0xff, 0xff, 0xff}}},
    {{{   130,      0,      0}, 0, {     0,      0}, {0xff, 0xff, 0xff, 0xff}}},
    {{{     0,      0,      0}, 0, {     0,      0}, {0xff, 0xff, 0xff, 0xff}}},
};

// 0x0200EDE8 - 0x0200EE28
const Gfx dl_draw_text_bg_box[] = {
    gsDPPipeSync(),
    gsSPClearGeometryMode(G_LIGHTING),
    gsDPSetCombineMode(G_CC_FADE, G_CC_FADE),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsSPVertex(vertex_text_bg_box, 4, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),
    gsSPEndDisplayList(),
};

#ifndef VERSION_EU
// 0x0200EE28 - 0x0200EE68
static const Vtx vertex_ia8_char[] = {
#ifndef VERSION_JP
    {{{     0,      0,      0}, 0, {     0,    256}, {0xff, 0xff, 0xff, 0xff}}},
    {{{     8,      0,      0}, 0, {     0,      0}, {0xff, 0xff, 0xff, 0xff}}},
    {{{     8,     16,      0}, 0, {   480,      0}, {0xff, 0xff, 0xff, 0xff}}},
    {{{     0,     16,      0}, 0, {   480,    256}, {0xff, 0xff, 0xff, 0xff}}},
#else
    {{{     0,      0,      0}, 0, {     0,   1024}, {0xff, 0xff, 0xff, 0xff}}},
    {{{     8,      0,      0}, 0, {   512,   1024}, {0xff, 0xff, 0xff, 0xff}}},
    {{{     8,     16,      0}, 0, {   512,      0}, {0xff, 0xff, 0xff, 0xff}}},
    {{{     0,     16,      0}, 0, {     0,      0}, {0xff, 0xff, 0xff, 0xff}}},
#endif
};
// !EU
#endif

#ifdef VERSION_EU
// 0x020073B0
const Gfx dl_ia_text_begin[] = {
    gsDPPipeSync(),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetCombineMode(G_CC_FADEA, G_CC_FADEA),
    gsDPSetEnvColor(255, 255, 255, 255),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetTextureFilter(G_TF_POINT),
    gsSPEndDisplayList(),
};

// 0x020073E8 - 0x02007418
const Gfx dl_ia_text_tex_settings[] = {
    gsDPSetTile(G_IM_FMT_IA, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_WRAP | G_TX_MIRROR, 3, G_TX_NOLOD, G_TX_WRAP | G_TX_MIRROR, 4, G_TX_NOLOD),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, ((16 * 8 + G_IM_SIZ_4b_INCR) >> G_IM_SIZ_4b_SHIFT) - 1, CALC_DXT(16, G_IM_SIZ_4b_BYTES)),
    gsDPSetTile(G_IM_FMT_IA, G_IM_SIZ_4b, 1, 0, G_TX_RENDERTILE, 0, G_TX_WRAP | G_TX_MIRROR, 3, G_TX_NOLOD, G_TX_WRAP | G_TX_MIRROR, 4, G_TX_NOLOD),
    gsDPSetTileSize(0, 0, 0, (16 - 1) << G_TEXTURE_IMAGE_FRAC, (8 - 1) << G_TEXTURE_IMAGE_FRAC),
    gsSPEndDisplayList(),
};

// 0x02007418 - 0x02007450
const Gfx dl_ia_text_end[] = {
    gsDPPipeSync(),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetRenderMode(G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsDPSetEnvColor(255, 255, 255, 255),
    gsDPSetTextureFilter(G_TF_BILERP),
    gsSPEndDisplayList(),
};

#elif defined(VERSION_US)
const Gfx dl_ia_text_begin[] = {
    gsDPPipeSync(),
    gsSPClearGeometryMode(G_LIGHTING),
    gsDPSetCombineMode(G_CC_FADEA, G_CC_FADEA),
    gsDPSetEnvColor(255, 255, 255, 255),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetTextureFilter(G_TF_POINT),
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON),
    gsSPEndDisplayList(),
};

const Gfx dl_ia_text_tex_settings[] = {
    gsDPSetTile(G_IM_FMT_IA, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, 3, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 4, G_TX_NOLOD),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, ((16 * 8 + G_IM_SIZ_4b_INCR) >> G_IM_SIZ_4b_SHIFT) - 1, CALC_DXT(16, G_IM_SIZ_4b_BYTES)),
    gsDPSetTile(G_IM_FMT_IA, G_IM_SIZ_4b, 1, 0, G_TX_RENDERTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, 3, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 4, G_TX_NOLOD),
    gsDPSetTileSize(0, 0, 0, (16 - 1) << G_TEXTURE_IMAGE_FRAC, (8 - 1) << G_TEXTURE_IMAGE_FRAC),
    gsSPVertex(vertex_ia8_char, 4, 0),
    gsSP2Triangles( 0,  1,  2, 0x0, 0,  2,  3, 0x0),
    gsSPEndDisplayList(),
};

#else
// 0x0200EE68 - 0x0200EEA8
const Gfx dl_ia_text_begin[] = {
    gsDPPipeSync(),
    gsSPClearGeometryMode(G_LIGHTING),
    gsDPSetCombineMode(G_CC_FADEA, G_CC_FADEA),
    gsDPSetEnvColor(255, 255, 255, 255),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetTextureFilter(G_TF_POINT),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsSPEndDisplayList(),
};

// 0x0200EEA8 - 0x0200EEF0
const Gfx dl_ia_text_tex_settings[] = {
    gsDPSetTile(G_IM_FMT_IA, G_IM_SIZ_8b, 0, 0, G_TX_LOADTILE, 0, G_TX_CLAMP, 4, G_TX_NOLOD, G_TX_CLAMP, 3, G_TX_NOLOD),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 8 * 16 - 1, CALC_DXT(8, G_IM_SIZ_8b_BYTES)),
    gsDPSetTile(G_IM_FMT_IA, G_IM_SIZ_8b, 1, 0, G_TX_RENDERTILE, 0, G_TX_CLAMP, 4, G_TX_NOLOD, G_TX_CLAMP, 3, G_TX_NOLOD),
    gsDPSetTileSize(0, 0, 0, (8 - 1) << G_TEXTURE_IMAGE_FRAC, (16 - 1) << G_TEXTURE_IMAGE_FRAC),
    gsSPVertex(vertex_ia8_char, 4, 0),
    gsSP2Triangles( 0,  1,  2, 0x0, 0,  2,  3, 0x0),
    gsSPEndDisplayList(),
};
#endif

#ifndef VERSION_EU
// 0x0200EEF0 - 0x0200EF30
const Gfx dl_ia_text_end[] = {
    gsDPPipeSync(),
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsDPSetEnvColor(255, 255, 255, 255),
    gsSPSetGeometryMode(G_LIGHTING | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2),
    gsDPSetTextureFilter(G_TF_BILERP),
    gsSPEndDisplayList(),
};
#endif

// 0x0200EF30 - 0x0200EF60
static const Vtx vertex_triangle[] = {
    {{{     0,      0,      0}, 0, {     0,      0}, {0xff, 0xff, 0xff, 0xff}}},
    {{{     8,      8,      0}, 0, {     0,      0}, {0xff, 0xff, 0xff, 0xff}}},
    {{{     0,     16,      0}, 0, {     0,      0}, {0xff, 0xff, 0xff, 0xff}}},
};

// 0x0200EF60 - 0x0200EFB0
const Gfx dl_draw_triangle[] = {
    gsSPClearGeometryMode(G_LIGHTING),
    gsDPSetCombineMode(G_CC_FADE, G_CC_FADE),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetTextureFilter(G_TF_POINT),
    gsSPVertex(vertex_triangle, 3, 0),
    gsSP1Triangle( 0,  1,  2, 0x0),
    gsSPSetGeometryMode(G_LIGHTING),
    gsDPSetRenderMode(G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsSPEndDisplayList(),
};

// 0x0200EFB0 - 0x0200EFF0
static const Vtx vertex_billboard_num[] = {
    {{{   -32,    -32,      0}, 0, {     0,   1024}, {0xff, 0xff, 0xff, 0xff}}},
    {{{    32,    -32,      0}, 0, {  1024,   1024}, {0xff, 0xff, 0xff, 0xff}}},
    {{{    32,     32,      0}, 0, {  1024,      0}, {0xff, 0xff, 0xff, 0xff}}},
    {{{   -32,     32,      0}, 0, {     0,      0}, {0xff, 0xff, 0xff, 0xff}}},
};

// 0x0200EFF0 - 0x0200F038
const Gfx dl_billboard_num_begin[] = {
    gsDPPipeSync(),
    gsDPSetCombineMode(G_CC_DECALRGBA, G_CC_DECALRGBA),
    gsSPClearGeometryMode(G_LIGHTING),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsDPTileSync(),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 4, 0, G_TX_RENDERTILE, 0, G_TX_CLAMP, 4, G_TX_NOLOD, G_TX_CLAMP, 4, G_TX_NOLOD),
    gsDPSetTileSize(0, 0, 0, (16 - 1) << G_TEXTURE_IMAGE_FRAC, (16 - 1) << G_TEXTURE_IMAGE_FRAC),
    gsSPEndDisplayList(),
};

// 0x0200F038 - 0x0200F078
const Gfx dl_billboard_num_end[] = {
    gsSPVertex(vertex_billboard_num, 4, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_OFF),
    gsDPPipeSync(),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsSPSetGeometryMode(G_LIGHTING),
    gsSPEndDisplayList(),
};

// 0x0200F078 - 0x0200F0A8
const Gfx dl_billboard_num_0[] = {
    gsSPDisplayList(dl_billboard_num_begin),
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texture_hud_char_0),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 16 * 16 - 1, CALC_DXT(16, G_IM_SIZ_16b_BYTES)),
    gsSPDisplayList(dl_billboard_num_end),
    gsSPEndDisplayList(),
};

// 0x0200F0A8 - 0x0200F0D8
const Gfx dl_billboard_num_1[] = {
    gsSPDisplayList(dl_billboard_num_begin),
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texture_hud_char_1),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 16 * 16 - 1, CALC_DXT(16, G_IM_SIZ_16b_BYTES)),
    gsSPDisplayList(dl_billboard_num_end),
    gsSPEndDisplayList(),
};

// 0x0200F0D8 - 0x0200F108
const Gfx dl_billboard_num_2[] = {
    gsSPDisplayList(dl_billboard_num_begin),
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texture_hud_char_2),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 16 * 16 - 1, CALC_DXT(16, G_IM_SIZ_16b_BYTES)),
    gsSPDisplayList(dl_billboard_num_end),
    gsSPEndDisplayList(),
};

// 0x0200F108 - 0x0200F138
const Gfx dl_billboard_num_3[] = {
    gsSPDisplayList(dl_billboard_num_begin),
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texture_hud_char_3),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 16 * 16 - 1, CALC_DXT(16, G_IM_SIZ_16b_BYTES)),
    gsSPDisplayList(dl_billboard_num_end),
    gsSPEndDisplayList(),
};

// 0x0200F138 - 0x0200F168
const Gfx dl_billboard_num_4[] = {
    gsSPDisplayList(dl_billboard_num_begin),
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texture_hud_char_4),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 16 * 16 - 1, CALC_DXT(16, G_IM_SIZ_16b_BYTES)),
    gsSPDisplayList(dl_billboard_num_end),
    gsSPEndDisplayList(),
};

// 0x0200F168 - 0x0200F198
const Gfx dl_billboard_num_5[] = {
    gsSPDisplayList(dl_billboard_num_begin),
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texture_hud_char_5),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 16 * 16 - 1, CALC_DXT(16, G_IM_SIZ_16b_BYTES)),
    gsSPDisplayList(dl_billboard_num_end),
    gsSPEndDisplayList(),
};

// 0x0200F198 - 0x0200F1C8
const Gfx dl_billboard_num_6[] = {
    gsSPDisplayList(dl_billboard_num_begin),
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texture_hud_char_6),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 16 * 16 - 1, CALC_DXT(16, G_IM_SIZ_16b_BYTES)),
    gsSPDisplayList(dl_billboard_num_end),
    gsSPEndDisplayList(),
};

// 0x0200F1C8 - 0x0200F1F8
const Gfx dl_billboard_num_7[] = {
    gsSPDisplayList(dl_billboard_num_begin),
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texture_hud_char_7),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 16 * 16 - 1, CALC_DXT(16, G_IM_SIZ_16b_BYTES)),
    gsSPDisplayList(dl_billboard_num_end),
    gsSPEndDisplayList(),
};

// 0x0200F1F8 - 0x0200F228
const Gfx dl_billboard_num_8[] = {
    gsSPDisplayList(dl_billboard_num_begin),
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texture_hud_char_8),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 16 * 16 - 1, CALC_DXT(16, G_IM_SIZ_16b_BYTES)),
    gsSPDisplayList(dl_billboard_num_end),
    gsSPEndDisplayList(),
};

// 0x0200F228 - 0x0200F258
const Gfx dl_billboard_num_9[] = {
    gsSPDisplayList(dl_billboard_num_begin),
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texture_hud_char_9),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 16 * 16 - 1, CALC_DXT(16, G_IM_SIZ_16b_BYTES)),
    gsSPDisplayList(dl_billboard_num_end),
    gsSPEndDisplayList(),
};

ALIGNED8 static const u8 texture_shadow_quarter_circle[] = "textures/segment2/shadow_quarter_circle.ia8";

ALIGNED8 static const u8 texture_shadow_quarter_square[] = "textures/segment2/shadow_quarter_square.ia8";

ALIGNED8 const u8 texture_transition_star_half[] = "textures/segment2/segment2.0F458.ia8";

ALIGNED8 const u8 texture_transition_circle_half[] = "textures/segment2/segment2.0FC58.ia8";

ALIGNED8 const u8 texture_transition_mario[] = "textures/segment2/segment2.10458.ia8";

ALIGNED8 const u8 texture_transition_bowser_half[] = "textures/segment2/segment2.11458.ia8";

ALIGNED8 const u8 texture_waterbox_water[] = "textures/segment2/segment2.11C58.rgba16";

ALIGNED8 const u8 texture_waterbox_jrb_water[] = "textures/segment2/segment2.12458.rgba16";

ALIGNED8 const u8 texture_waterbox_unknown_water[] = "textures/segment2/segment2.12C58.rgba16";

ALIGNED8 const u8 texture_waterbox_mist[] = "textures/segment2/segment2.13458.ia16";

ALIGNED8 const u8 texture_waterbox_lava[] = "textures/segment2/segment2.13C58.rgba16";

// Unreferenced light group
static const Lights1 segment2_lights_unused = gdSPDefLights1(
    0x40, 0x40, 0x40,
    0xff, 0xff, 0xff, 0x28, 0x28, 0x28
);

// 0x02014470 - 0x020144B0
const Mtx matrix_identity = {
    {{1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f}}
};


// 0x020144B0 - 0x020144F0
static const Mtx matrix_fullscreen = {
    {{2.0f / SCREEN_WIDTH, 0.0f, 0.0f, 0.0f},
    {0.0f, 2.0f / SCREEN_HEIGHT, 0.0f, 0.0f},
    {0.0f, 0.0f, -1.0f, 0.0f},
    {-1.0f, -1.0f, -1.0f, 1.0f}}
};


// 0x020144F0 - 0x02014508
const Gfx dl_draw_quad_verts_0123[] = {
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),
    gsSPEndDisplayList(),
};

// 0x02014508 - 0x02014520
const Gfx dl_draw_quad_verts_4567[] = {
    gsSP2Triangles( 4,  5,  6, 0x0,  4,  6,  7, 0x0),
    gsSPEndDisplayList(),
};

const Gfx dl_shadow_begin[] = {
    gsDPPipeSync(),
    gsSPClearGeometryMode(G_LIGHTING | G_CULL_BACK),
    gsDPSetCombineMode(G_CC_MODULATEIA, G_CC_MODULATEIA),
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON),
    gsSPEndDisplayList(),
};

const Gfx dl_shadow_circle[] = {
    gsSPDisplayList(dl_shadow_begin),
    gsDPLoadTextureBlock(texture_shadow_quarter_circle, G_IM_FMT_IA, G_IM_SIZ_8b, 16, 16, 0, G_TX_WRAP | G_TX_MIRROR, G_TX_WRAP | G_TX_MIRROR, 4, 4, G_TX_NOLOD, G_TX_NOLOD),
    gsSPEndDisplayList(),
};

const Gfx dl_shadow_square[] = {
    gsSPDisplayList(dl_shadow_begin),
    gsDPLoadTextureBlock(texture_shadow_quarter_square, G_IM_FMT_IA, G_IM_SIZ_8b, 16, 16, 0, G_TX_WRAP | G_TX_MIRROR, G_TX_WRAP | G_TX_MIRROR, 4, 4, G_TX_NOLOD, G_TX_NOLOD),
    gsSPEndDisplayList(),
};

// 0x020145D8 - 0x02014620
const Gfx dl_shadow_9_verts[] = {
    gsSP2Triangles( 0,  3,  4, 0x0,  0,  4,  1, 0x0),
    gsSP2Triangles( 1,  4,  2, 0x0,  2,  4,  5, 0x0),
    gsSP2Triangles( 3,  6,  4, 0x0,  4,  6,  7, 0x0),
    gsSP2Triangles( 4,  7,  8, 0x0,  4,  8,  5, 0x0),
    gsSPEndDisplayList(),
};

// 0x02014620 - 0x02014638
const Gfx dl_shadow_4_verts[] = {
    gsSP2Triangles( 0,  2,  1, 0x0,  1,  2,  3, 0x0),
    gsSPEndDisplayList(),
};

// 0x02014638 - 0x02014660
const Gfx dl_shadow_end[] = {
    gsDPPipeSync(),
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF),
    gsSPSetGeometryMode(G_LIGHTING | G_CULL_BACK),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsSPEndDisplayList(),
};

// 0x02014660 - 0x02014698
const Gfx dl_proj_mtx_fullscreen[] = {
    gsDPPipeSync(),
    gsSPClearGeometryMode(G_LIGHTING),
    gsSPMatrix(&matrix_identity, G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH),
    gsSPMatrix(&matrix_fullscreen, G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH),
    gsSPMatrix(&matrix_identity, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH),
    gsSPPerspNormalize(0xFFFF),
    gsSPEndDisplayList(),
};

// 0x02014698 - 0x020146C0
const Gfx dl_screen_transition_end[] = {
    gsDPPipeSync(),
    gsSPSetGeometryMode(G_LIGHTING),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsDPSetRenderMode(G_RM_OPA_SURF, G_RM_OPA_SURF2),
    gsSPEndDisplayList(),
};

// 0x020146C0 - 0x02014708
const Gfx dl_transition_draw_filled_region[] = {
    gsSP2Triangles( 0,  4,  1, 0x0,  1,  4,  5, 0x0),
    gsSP2Triangles( 1,  5,  2, 0x0,  2,  5,  6, 0x0),
    gsSP2Triangles( 2,  6,  7, 0x0,  2,  7,  3, 0x0),
    gsSP2Triangles( 3,  4,  0, 0x0,  3,  7,  4, 0x0),
    gsSPEndDisplayList(),
};

// 0x02014708 - 0x02014738
const Gfx dl_skybox_begin[] = {
    gsDPPipeSync(),
    gsSPClearGeometryMode(G_LIGHTING),
    gsDPSetCombineMode(G_CC_MODULATERGB, G_CC_MODULATERGB),
    gsSPPerspNormalize(0xFFFF),
    gsSPMatrix(&matrix_identity, G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH),
    gsSPEndDisplayList(),
};

// 0x02014768 - 0x02014790
const Gfx dl_skybox_end[] = {
    gsDPPipeSync(),
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF),
    gsSPSetGeometryMode(G_LIGHTING),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsSPEndDisplayList(),
};

// 0x02014790 - 0x020147D0
const Gfx dl_waterbox_rgba16_begin[] = {
    gsDPPipeSync(),
    gsDPSetCombineMode(G_CC_MODULATERGBA, G_CC_MODULATERGBA),
    gsSPClearGeometryMode(G_LIGHTING | G_CULL_BACK),
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON),
    gsDPTileSync(),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, 0, G_TX_RENDERTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD),
    gsDPSetTileSize(0, 0, 0, (32 - 1) << G_TEXTURE_IMAGE_FRAC, (32 - 1) << G_TEXTURE_IMAGE_FRAC),
    gsSPEndDisplayList(),
};

// 0x020147D0 - 0x02014810
const Gfx dl_waterbox_ia16_begin[] = {
    gsDPPipeSync(),
    gsDPSetCombineMode(G_CC_MODULATEIA, G_CC_MODULATEIA),
    gsSPClearGeometryMode(G_LIGHTING | G_CULL_BACK),
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON),
    gsDPTileSync(),
    gsDPSetTile(G_IM_FMT_IA, G_IM_SIZ_16b, 8, 0, G_TX_RENDERTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD),
    gsDPSetTileSize(0, 0, 0, (32 - 1) << G_TEXTURE_IMAGE_FRAC, (32 - 1) << G_TEXTURE_IMAGE_FRAC),
    gsSPEndDisplayList(),
};

// 0x02014810 - 0x02014838
const Gfx dl_waterbox_end[] = {
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF),
    gsDPPipeSync(),
    gsSPSetGeometryMode(G_LIGHTING | G_CULL_BACK),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsSPEndDisplayList(),
};

// 0x02014838 - 0x02014878
ALIGNED8 static const u8 texture_ia8_up_arrow[] = "textures/segment2/segment2.14838.ia8";

// 0x02014878 - 0x020148B0
const Gfx dl_ia8_up_arrow_begin[] = {
    gsDPPipeSync(),
    gsSPClearGeometryMode(G_LIGHTING),
    gsDPSetCombineMode(G_CC_MODULATEIA, G_CC_MODULATEIA),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_NOOP2),
    gsSPPerspNormalize(0xFFFF),
    gsSPMatrix(&matrix_identity, G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH),
    gsSPEndDisplayList(),
};

// 0x020148B0 - 0x020148E0
// Unused, seems to be an early DL for the power meter, seeing that is loading a 64x32 texture
const Gfx dl_rgba16_unused[] = {
    gsSPMatrix(&matrix_identity, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH),
    gsDPTileSync(),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 16, 0, G_TX_RENDERTILE, 0, G_TX_CLAMP, 5, G_TX_NOLOD, G_TX_CLAMP, 6, G_TX_NOLOD),
    gsDPSetTileSize(0, 0, 0, (64 - 1) << G_TEXTURE_IMAGE_FRAC, (32 - 1) << G_TEXTURE_IMAGE_FRAC),
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON),
    gsSPEndDisplayList(),
};

// 0x020148E0 - 0x02014938
const Gfx dl_ia8_up_arrow_load_texture_block[] = {
    gsDPPipeSync(),
    gsDPSetCombineMode(G_CC_MODULATEIA, G_CC_MODULATEIA),
    gsDPTileSync(),
    gsDPSetTile(G_IM_FMT_IA, G_IM_SIZ_8b, 1, 0, G_TX_RENDERTILE, 0, G_TX_CLAMP, 3, G_TX_NOLOD, G_TX_CLAMP, 3, G_TX_NOLOD),
    gsDPSetTileSize(0, 0, 0, (8 - 1) << G_TEXTURE_IMAGE_FRAC, (8 - 1) << G_TEXTURE_IMAGE_FRAC),
    gsDPSetTextureImage(G_IM_FMT_IA, G_IM_SIZ_8b, 1, texture_ia8_up_arrow),
    gsDPTileSync(),
    gsDPSetTile(G_IM_FMT_IA, G_IM_SIZ_8b, 0, 0, G_TX_LOADTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 8 * 8 - 1, CALC_DXT(8, G_IM_SIZ_8b_BYTES)),
    gsSPEndDisplayList(),
};

// 0x02014938 - 0x02014958
const Gfx dl_ia8_up_arrow_end[] = {
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF),
    gsDPPipeSync(),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsSPEndDisplayList(),
};

// 0x02014958 - 0x02014960
static const Lights1 seg2_lights_02014958 = gdSPDefLights1(
    0x50, 0x50, 0x50,
    0xff, 0xff, 0xff, 0x32, 0x32, 0x32
);

// 0x02014970 - 0x020149A8
const Gfx dl_paintings_rippling_begin[] = {
    gsDPPipeSync(),
    gsSPSetGeometryMode(G_LIGHTING | G_SHADING_SMOOTH),
    gsDPSetCombineMode(G_CC_MODULATERGBA, G_CC_MODULATERGBA),
    gsSPLight(&seg2_lights_02014958.l, 1),
    gsSPLight(&seg2_lights_02014958.a, 2),
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON),
    gsSPEndDisplayList(),
};

// 0x020149A8 - 0x020149C8
const Gfx dl_paintings_rippling_end[] = {
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF),
    gsDPPipeSync(),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsSPEndDisplayList(),
};

// 0x020149C8 - 0x02014A00
const Gfx dl_paintings_env_mapped_begin[] = {
    gsDPPipeSync(),
    gsSPSetGeometryMode(G_LIGHTING | G_TEXTURE_GEN),
    gsDPSetCombineMode(G_CC_DECALRGB, G_CC_DECALRGB),
    gsSPLight(&seg2_lights_02014958.l, 1),
    gsSPLight(&seg2_lights_02014958.a, 2),
    gsSPTexture(0x4000, 0x4000, 0, G_TX_RENDERTILE, G_ON),
    gsSPEndDisplayList(),
};

// 0x02014A00 - 0x02014A30
const Gfx dl_paintings_env_mapped_end[] = {
    gsSPTexture(0x4000, 0x4000, 0, G_TX_RENDERTILE, G_OFF),
    gsDPPipeSync(),
    gsSPGeometryModeSetFirst(G_TEXTURE_GEN, G_LIGHTING),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsSPEndDisplayList(),
};

// 0x02014A30 - 0x02014A60
const Gfx dl_paintings_draw_ripples[] = {
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 6,  7,  8, 0x0,  9, 10, 11, 0x0),
    gsSP1Triangle(12, 13, 14, 0x0),
    gsSPEndDisplayList(),
};

// 14A60: triangle mesh
// 0x02014A60
const s16 seg2_painting_triangle_mesh[] = {
    157, // numVtx
    // format:
    // 2D point (x, y), ripple (0 or 1)
    614, 583,   0, // 0
    614, 614,   0, // 1
    562, 614,   0, // 2
    562, 553,   1, // 3
    614, 522,   0, // 4
    511, 583,   1, // 5
    511, 614,   0, // 6
    307, 614,   0, // 7
    307, 583,   1, // 8
    358, 614,   0, // 9
    256, 614,   0, // 10
    256, 553,   1, // 11
    307, 522,   1, // 12
    358, 553,   1, // 13
    409, 583,   1, // 14
    460, 614,   0, // 15
    511, 522,   1, // 16
    460, 553,   1, // 17
    409, 522,   1, // 18
    562, 307,   1, // 19
    614, 338,   0, // 20
    562, 430,   1, // 21
    614, 399,   0, // 22
    562, 368,   1, // 23
    511, 338,   1, // 24
    460, 307,   1, // 25
    460, 430,   1, // 26
    511, 399,   1, // 27
    511, 460,   1, // 28
    409, 338,   1, // 29
    460, 368,   1, // 30
    358, 307,   1, // 31
    409, 460,   1, // 32
    358, 430,   1, // 33
    409, 399,   1, // 34
    358, 368,   1, // 35
    307, 338,   1, // 36
    256, 307,   1, // 37
    307, 399,   1, // 38
    256, 430,   1, // 39
    307, 460,   1, // 40
    614, 460,   0, // 41
    562, 491,   1, // 42
    460, 491,   1, // 43
    358, 491,   1, // 44
    256, 491,   1, // 45
    409, 276,   1, // 46
    511, 276,   1, // 47
    307, 276,   1, // 48
    614,  31,   0, // 49
    614,   0,   0, // 50
    562,   0,   0, // 51
    562, 123,   1, // 52
    614,  92,   0, // 53
    511,  31,   1, // 54
    562,  61,   1, // 55
    460,   0,   0, // 56
    511,   0,   0, // 57
    460, 123,   1, // 58
    511,  92,   1, // 59
    511, 153,   1, // 60
    409,  31,   1, // 61
    460,  61,   1, // 62
    358,   0,   0, // 63
    409,   0,   0, // 64
    409,  92,   1, // 65
    358, 123,   1, // 66
    409, 153,   1, // 67
    307,  31,   1, // 68
    358,  61,   1, // 69
    256,   0,   0, // 70
    307,   0,   0, // 71
    256, 123,   1, // 72
    307,  92,   1, // 73
    307, 153,   1, // 74
    614, 153,   0, // 75
    562, 246,   1, // 76
    614, 215,   0, // 77
    562, 184,   1, // 78
    460, 246,   1, // 79
    511, 215,   1, // 80
    460, 184,   1, // 81
    358, 246,   1, // 82
    409, 215,   1, // 83
    358, 184,   1, // 84
    256, 246,   1, // 85
    307, 215,   1, // 86
    205, 583,   1, // 87
      0, 614,   0, // 88
      0, 583,   0, // 89
     51, 614,   0, // 90
     51, 553,   1, // 91
    102, 583,   1, // 92
    205, 522,   1, // 93
    153, 553,   1, // 94
    153, 614,   0, // 95
    102, 522,   1, // 96
    256, 368,   1, // 97
    205, 338,   1, // 98
    153, 307,   1, // 99
    153, 430,   1, // 100
    205, 399,   1, // 101
    205, 460,   1, // 102
    153, 368,   1, // 103
    102, 338,   1, // 104
     51, 307,   1, // 105
     51, 430,   1, // 106
    102, 399,   1, // 107
    102, 460,   1, // 108
     51, 368,   1, // 109
      0, 338,   0, // 110
      0, 460,   0, // 111
    153, 491,   1, // 112
     51, 491,   1, // 113
    153, 246,   1, // 114
    102, 276,   1, // 115
    205, 276,   1, // 116
      0, 276,   0, // 117
     51, 246,   1, // 118
    205,  31,   1, // 119
    256,  61,   1, // 120
    205,   0,   0, // 121
    153,   0,   0, // 122
    205, 153,   1, // 123
    205,  92,   1, // 124
    153, 123,   1, // 125
    102,  31,   1, // 126
    153,  61,   1, // 127
    102,   0,   0, // 128
     51,   0,   0, // 129
     51, 123,   1, // 130
    102,  92,   1, // 131
    102, 153,   1, // 132
      0,  31,   0, // 133
     51,  61,   1, // 134
      0, 153,   0, // 135
    256, 184,   1, // 136
    205, 215,   1, // 137
    153, 184,   1, // 138
    102, 215,   1, // 139
     51, 184,   1, // 140
    409, 614,   0, // 141
    614, 307,   0, // 142
    614, 276,   0, // 143
    511, 307,   1, // 144
    409, 307,   1, // 145
    307, 307,   1, // 146
    205, 614,   0, // 147
      0, 522,   0, // 148
    102, 614,   0, // 149
    205, 307,   1, // 150
    102, 307,   1, // 151
      0, 399,   0, // 152
      0, 307,   0, // 153
      0, 215,   0, // 154
      0,  92,   0, // 155
      0,   0,   0, // 156
    // triangles
    264,
      8,  12,  13, // 0
      0,   1,   2, // 1
      3,   0,   2, // 2
      4,   0,   3, // 3
      5,   2,   6, // 4
      2,   5,   3, // 5
      7,   8,   9, // 6
      8,   7,  10, // 7
     11,   8,  10, // 8
     12,   8,  11, // 9
      9,   8,  13, // 10
     13,  14,   9, // 11
     14, 141,   9, // 12
      5,   6,  15, // 13
      5,  16,   3, // 14
     16,   5,  17, // 15
     17,   5,  15, // 16
     14,  15, 141, // 17
     15,  14,  17, // 18
     18,  14,  13, // 19
     14,  18,  17, // 20
     19, 142,  20, // 21
     19,  20,  23, // 22
     28,  27,  21, // 23
     21,  23,  22, // 24
     22,  41,  21, // 25
     20,  22,  23, // 26
     23,  24,  19, // 27
     21,  27,  23, // 28
     24,  23,  27, // 29
     25, 144,  24, // 30
     19,  24, 144, // 31
     24,  27,  30, // 32
     25,  24,  30, // 33
     26,  30,  27, // 34
     27,  28,  26, // 35
     36,  38,  97, // 36
     26,  34,  30, // 37
     29,  30,  34, // 38
     30,  29,  25, // 39
     25,  29, 145, // 40
     31, 145,  29, // 41
     31,  29,  35, // 42
     29,  34,  35, // 43
     32,  34,  26, // 44
     33,  35,  34, // 45
     34,  32,  33, // 46
     33,  38,  35, // 47
     35,  36,  31, // 48
     36,  35,  38, // 49
     37,  36,  97, // 50
     37, 146,  36, // 51
     31,  36, 146, // 52
     28,  16,  43, // 53
     38,  40,  39, // 54
     39,  97,  38, // 55
     40,  38,  33, // 56
     21,  41,  42, // 57
     41,   4,  42, // 58
      3,  42,   4, // 59
     42,  28,  21, // 60
     28,  42,  16, // 61
      3,  16,  42, // 62
     26,  28,  43, // 63
     17,  43,  16, // 64
     43,  32,  26, // 65
     32,  43,  18, // 66
     17,  18,  43, // 67
     33,  32,  44, // 68
     32,  18,  44, // 69
     13,  44,  18, // 70
     44,  40,  33, // 71
     13,  12,  44, // 72
     40,  44,  12, // 73
     39,  40,  45, // 74
     40,  12,  45, // 75
     48,  31, 146, // 76
     11,  45,  12, // 77
     25,  47, 144, // 78
     46,  25, 145, // 79
     47,  19, 144, // 80
     19, 143, 142, // 81
     31,  46, 145, // 82
     60,  59,  52, // 83
     49,  53,  55, // 84
     50,  49,  51, // 85
     51,  49,  55, // 86
     52,  55,  53, // 87
     53,  75,  52, // 88
     54,  55,  59, // 89
     52,  59,  55, // 90
     55,  54,  51, // 91
     54,  59,  62, // 92
     56,  54,  62, // 93
     57,  54,  56, // 94
     54,  57,  51, // 95
     58,  62,  59, // 96
     59,  60,  58, // 97
     68,  71,  63, // 98
     61,  62,  65, // 99
     58,  65,  62, // 100
     62,  61,  56, // 101
     61,  65,  69, // 102
     63,  61,  69, // 103
     64,  61,  63, // 104
     61,  64,  56, // 105
     65,  67,  66, // 106
     66,  69,  65, // 107
     67,  65,  58, // 108
     68,  69,  73, // 109
     69,  68,  63, // 110
     66,  73,  69, // 111
     68,  73, 120, // 112
     70,  68, 120, // 113
     71,  68,  70, // 114
     72, 120,  73, // 115
     73,  74,  72, // 116
     74,  73,  66, // 117
     75,  77,  78, // 118
     52,  75,  78, // 119
     76,  78,  77, // 120
     77, 143,  76, // 121
     76,  80,  78, // 122
     60,  78,  80, // 123
     78,  60,  52, // 124
     46,  83,  79, // 125
     58,  60,  81, // 126
     60,  80,  81, // 127
     79,  81,  80, // 128
     80,  47,  79, // 129
     47,  80,  76, // 130
     81,  67,  58, // 131
     67,  81,  83, // 132
     79,  83,  81, // 133
     66,  67,  84, // 134
     67,  83,  84, // 135
     82,  84,  83, // 136
     83,  46,  82, // 137
     84,  74,  66, // 138
     82,  86,  84, // 139
     74,  84,  86, // 140
     74,  86, 136, // 141
     72,  74, 136, // 142
     85, 136,  86, // 143
     86,  48,  85, // 144
     48,  86,  82, // 145
     25,  46,  79, // 146
     79,  47,  25, // 147
     82,  46,  31, // 148
     19,  47,  76, // 149
     76, 143,  19, // 150
     31,  48,  82, // 151
     37,  48, 146, // 152
     85,  48,  37, // 153
     10,  87,  11, // 154
     87,  10, 147, // 155
     92,  95, 149, // 156
     88,  89,  90, // 157
     89, 148,  91, // 158
     90,  89,  91, // 159
     91,  92,  90, // 160
     92, 149,  90, // 161
     93,  87,  94, // 162
     87,  93,  11, // 163
     94,  87,  95, // 164
     87, 147,  95, // 165
     95,  92,  94, // 166
     96,  92,  91, // 167
     92,  96,  94, // 168
     39, 101,  97, // 169
     97,  98,  37, // 170
     98,  97, 101, // 171
     99,  98, 103, // 172
     99, 150,  98, // 173
     37,  98, 150, // 174
     98, 101, 103, // 175
    100, 103, 101, // 176
    101, 102, 100, // 177
    102, 101,  39, // 178
    100, 107, 103, // 179
    103, 104,  99, // 180
    104, 103, 107, // 181
    105, 104, 109, // 182
    105, 151, 104, // 183
     99, 104, 151, // 184
    104, 107, 109, // 185
    106, 109, 107, // 186
    107, 108, 106, // 187
    108, 107, 100, // 188
    109, 110, 105, // 189
    106, 152, 109, // 190
    110, 109, 152, // 191
    105, 110, 153, // 192
    111, 152, 106, // 193
     11,  93,  45, // 194
    102,  45,  93, // 195
     45, 102,  39, // 196
    102,  93, 112, // 197
    100, 102, 112, // 198
     94, 112,  93, // 199
    112, 108, 100, // 200
    108, 112,  96, // 201
     94,  96, 112, // 202
    106, 108, 113, // 203
    108,  96, 113, // 204
     91, 113,  96, // 205
     91, 148, 113, // 206
    113, 111, 106, // 207
    111, 113, 148, // 208
    114, 116,  99, // 209
     99, 115, 114, // 210
    115,  99, 151, // 211
     99, 116, 150, // 212
     72, 124, 120, // 213
    116,  37, 150, // 214
     37, 116,  85, // 215
    117, 105, 153, // 216
    105, 115, 151, // 217
    105, 117, 118, // 218
    118, 115, 105, // 219
    119, 120, 124, // 220
    120, 119,  70, // 221
    119, 124, 127, // 222
    119, 121,  70, // 223
    121, 119, 122, // 224
    122, 119, 127, // 225
    123, 124,  72, // 226
    124, 123, 125, // 227
    125, 127, 124, // 228
    126, 127, 131, // 229
    127, 126, 122, // 230
    125, 131, 127, // 231
    126, 131, 134, // 232
    128, 126, 129, // 233
    129, 126, 134, // 234
    126, 128, 122, // 235
    136, 123,  72, // 236
    130, 134, 131, // 237
    131, 132, 130, // 238
    132, 131, 125, // 239
    133, 134, 155, // 240
    134, 133, 129, // 241
    130, 155, 134, // 242
    133, 156, 129, // 243
    135, 155, 130, // 244
    123, 136, 137, // 245
     85, 137, 136, // 246
    139, 115, 118, // 247
    123, 137, 138, // 248
    125, 123, 138, // 249
    114, 138, 137, // 250
    137, 116, 114, // 251
    116, 137,  85, // 252
    114, 139, 138, // 253
    132, 138, 139, // 254
    138, 132, 125, // 255
    132, 139, 140, // 256
    130, 132, 140, // 257
    115, 139, 114, // 258
    118, 140, 139, // 259
    135, 140, 154, // 260
    118, 154, 140, // 261
    140, 135, 130, // 262
    117, 154, 118, // 263
};

/* 0x02015444: seg2_painting_mesh_neighbor_tris
 * Lists the neighboring triangles for each vertex in the mesh.
 * Used when applying gouraud shading to the generated ripple mesh
 *
 * Format:
 *      num neighbors, neighbor0, neighbor1, ...
 * The nth entry corresponds to the nth vertex in seg2_painting_triangle_mesh
 */
const s16 seg2_painting_mesh_neighbor_tris[] = {
      3,   1,   2,   3,
      1,   1,
      4,   1,   2,   4,   5,
      6,   2,   3,   5,  14,  59,  62,
      3,   3,  58,  59,
      6,   4,   5,  13,  14,  15,  16,
      2,   4,  13,
      2,   6,   7,
      6,   0,   6,   7,   8,   9,  10,
      4,   6,  10,  11,  12,
      4,   7,   8, 154, 155,
      6,   8,   9,  77, 154, 163, 194,
      6,   0,   9,  72,  73,  75,  77,
      6,   0,  10,  11,  19,  70,  72,
      6,  11,  12,  17,  18,  19,  20,
      4,  13,  16,  17,  18,
      6,  14,  15,  53,  61,  62,  64,
      6,  15,  16,  18,  20,  64,  67,
      6,  19,  20,  66,  67,  69,  70,
      8,  21,  22,  27,  31,  80,  81, 149, 150,
      3,  21,  22,  26,
      6,  23,  24,  25,  28,  57,  60,
      3,  24,  25,  26,
      6,  22,  24,  26,  27,  28,  29,
      6,  27,  29,  30,  31,  32,  33,
      8,  30,  33,  39,  40,  78,  79, 146, 147,
      6,  34,  35,  37,  44,  63,  65,
      6,  23,  28,  29,  32,  34,  35,
      6,  23,  35,  53,  60,  61,  63,
      6,  38,  39,  40,  41,  42,  43,
      6,  32,  33,  34,  37,  38,  39,
      8,  41,  42,  48,  52,  76,  82, 148, 151,
      6,  44,  46,  65,  66,  68,  69,
      6,  45,  46,  47,  56,  68,  71,
      6,  37,  38,  43,  44,  45,  46,
      6,  42,  43,  45,  47,  48,  49,
      6,  36,  48,  49,  50,  51,  52,
      8,  50,  51, 152, 153, 170, 174, 214, 215,
      6,  36,  47,  49,  54,  55,  56,
      6,  54,  55,  74, 169, 178, 196,
      6,  54,  56,  71,  73,  74,  75,
      3,  25,  57,  58,
      6,  57,  58,  59,  60,  61,  62,
      6,  53,  63,  64,  65,  66,  67,
      6,  68,  69,  70,  71,  72,  73,
      6,  74,  75,  77, 194, 195, 196,
      6,  79,  82, 125, 137, 146, 148,
      6,  78,  80, 129, 130, 147, 149,
      6,  76, 144, 145, 151, 152, 153,
      3,  84,  85,  86,
      1,  85,
      4,  85,  86,  91,  95,
      6,  83,  87,  88,  90, 119, 124,
      3,  84,  87,  88,
      6,  89,  91,  92,  93,  94,  95,
      6,  84,  86,  87,  89,  90,  91,
      4,  93,  94, 101, 105,
      2,  94,  95,
      6,  96,  97, 100, 108, 126, 131,
      6,  83,  89,  90,  92,  96,  97,
      6,  83,  97, 123, 124, 126, 127,
      6,  99, 101, 102, 103, 104, 105,
      6,  92,  93,  96,  99, 100, 101,
      4,  98, 103, 104, 110,
      2, 104, 105,
      6,  99, 100, 102, 106, 107, 108,
      6, 106, 107, 111, 117, 134, 138,
      6, 106, 108, 131, 132, 134, 135,
      6,  98, 109, 110, 112, 113, 114,
      6, 102, 103, 107, 109, 110, 111,
      4, 113, 114, 221, 223,
      2,  98, 114,
      6, 115, 116, 142, 213, 226, 236,
      6, 109, 111, 112, 115, 116, 117,
      6, 116, 117, 138, 140, 141, 142,
      3,  88, 118, 119,
      6, 120, 121, 122, 130, 149, 150,
      3, 118, 120, 121,
      6, 118, 119, 120, 122, 123, 124,
      6, 125, 128, 129, 133, 146, 147,
      6, 122, 123, 127, 128, 129, 130,
      6, 126, 127, 128, 131, 132, 133,
      6, 136, 137, 139, 145, 148, 151,
      6, 125, 132, 133, 135, 136, 137,
      6, 134, 135, 136, 138, 139, 140,
      6, 143, 144, 153, 215, 246, 252,
      6, 139, 140, 141, 143, 144, 145,
      6, 154, 155, 162, 163, 164, 165,
      1, 157,
      3, 157, 158, 159,
      4, 157, 159, 160, 161,
      6, 158, 159, 160, 167, 205, 206,
      6, 156, 160, 161, 166, 167, 168,
      6, 162, 163, 194, 195, 197, 199,
      6, 162, 164, 166, 168, 199, 202,
      4, 156, 164, 165, 166,
      6, 167, 168, 201, 202, 204, 205,
      6,  36,  50,  55, 169, 170, 171,
      6, 170, 171, 172, 173, 174, 175,
      8, 172, 173, 180, 184, 209, 210, 211, 212,
      6, 176, 177, 179, 188, 198, 200,
      6, 169, 171, 175, 176, 177, 178,
      6, 177, 178, 195, 196, 197, 198,
      6, 172, 175, 176, 179, 180, 181,
      6, 180, 181, 182, 183, 184, 185,
      8, 182, 183, 189, 192, 216, 217, 218, 219,
      6, 186, 187, 190, 193, 203, 207,
      6, 179, 181, 185, 186, 187, 188,
      6, 187, 188, 200, 201, 203, 204,
      6, 182, 185, 186, 189, 190, 191,
      3, 189, 191, 192,
      3, 193, 207, 208,
      6, 197, 198, 199, 200, 201, 202,
      6, 203, 204, 205, 206, 207, 208,
      6, 209, 210, 250, 251, 253, 258,
      6, 210, 211, 217, 219, 247, 258,
      6, 209, 212, 214, 215, 251, 252,
      3, 216, 218, 263,
      6, 218, 219, 247, 259, 261, 263,
      6, 220, 221, 222, 223, 224, 225,
      6, 112, 113, 115, 213, 220, 221,
      2, 223, 224,
      4, 224, 225, 230, 235,
      6, 226, 227, 236, 245, 248, 249,
      6, 213, 220, 222, 226, 227, 228,
      6, 227, 228, 231, 239, 249, 255,
      6, 229, 230, 232, 233, 234, 235,
      6, 222, 225, 228, 229, 230, 231,
      2, 233, 235,
      4, 233, 234, 241, 243,
      6, 237, 238, 242, 244, 257, 262,
      6, 229, 231, 232, 237, 238, 239,
      6, 238, 239, 254, 255, 256, 257,
      3, 240, 241, 243,
      6, 232, 234, 237, 240, 241, 242,
      3, 244, 260, 262,
      6, 141, 142, 143, 236, 245, 246,
      6, 245, 246, 248, 250, 251, 252,
      6, 248, 249, 250, 253, 254, 255,
      6, 247, 253, 254, 256, 258, 259,
      6, 256, 257, 259, 260, 261, 262,
      2,  12,  17,
      2,  21,  81,
      3,  81, 121, 150,
      4,  30,  31,  78,  80,
      4,  40,  41,  79,  82,
      4,  51,  52,  76, 152,
      2, 155, 165,
      3, 158, 206, 208,
      2, 156, 161,
      4, 173, 174, 212, 214,
      4, 183, 184, 211, 217,
      3, 190, 191, 193,
      2, 192, 216,
      3, 260, 261, 263,
      3, 240, 242, 244,
      1, 243,
};