#ifndef GFX_RT64_GEO_MAP_H
#define GFX_RT64_GEO_MAP_H

#include <map>
#include <string>

#include "actors/group0.h"
#include "actors/group1.h"
#include "actors/group2.h"
#include "actors/group3.h"
#include "actors/group4.h"
#include "actors/group5.h"
#include "actors/group6.h"
#include "actors/group7.h"
#include "actors/group8.h"
#include "actors/group9.h"
#include "actors/group10.h"
#include "actors/group11.h"
#include "actors/group12.h"
#include "actors/group13.h"
#include "actors/group14.h"
#include "actors/group15.h"
#include "actors/group16.h"
#include "actors/group17.h"
#include "actors/common0.h"
#include "actors/common1.h"

#define FILL_GEO_MAPS(x) geoNameMap[(void *)(x)] = #x; nameGeoMap[#x] = (void *)(x)

inline void gfx_rt64_init_geo_layout_maps(std::unordered_map<void *, std::string> &geoNameMap, std::map<std::string, void *> &nameGeoMap) {
    FILL_GEO_MAPS(mario_geo);
    FILL_GEO_MAPS(amp_geo);
    FILL_GEO_MAPS(birds_geo);
    FILL_GEO_MAPS(blargg_geo);
    FILL_GEO_MAPS(blue_coin_switch_geo);
    FILL_GEO_MAPS(fish_shadow_geo);
    FILL_GEO_MAPS(fish_geo);
    FILL_GEO_MAPS(black_bobomb_geo);
    FILL_GEO_MAPS(bobomb_buddy_geo);
    FILL_GEO_MAPS(bowser_bomb_geo);
    FILL_GEO_MAPS(boo_geo);
    FILL_GEO_MAPS(boo_castle_geo);
    FILL_GEO_MAPS(bookend_geo);
    FILL_GEO_MAPS(bookend_part_geo);
    FILL_GEO_MAPS(bowling_ball_geo);
    FILL_GEO_MAPS(bowling_ball_track_geo);
    FILL_GEO_MAPS(bowser_geo);
    FILL_GEO_MAPS(bowser2_geo);
    FILL_GEO_MAPS(bowser_flames_geo);
    FILL_GEO_MAPS(bowser_key_geo);
    FILL_GEO_MAPS(bowser_key_cutscene_geo);
    FILL_GEO_MAPS(breakable_box_geo);
    FILL_GEO_MAPS(breakable_box_small_geo);
    FILL_GEO_MAPS(bub_geo);
    FILL_GEO_MAPS(bubba_geo);
    FILL_GEO_MAPS(bubble_geo);
    FILL_GEO_MAPS(purple_marble_geo);
    FILL_GEO_MAPS(bullet_bill_geo);
    FILL_GEO_MAPS(bully_geo);
    FILL_GEO_MAPS(bully_boss_geo);
    FILL_GEO_MAPS(burn_smoke_geo);
    FILL_GEO_MAPS(butterfly_geo);
    FILL_GEO_MAPS(cannon_barrel_geo);
    FILL_GEO_MAPS(cannon_base_geo);
    FILL_GEO_MAPS(cap_switch_geo);
    FILL_GEO_MAPS(metallic_ball_geo);
    FILL_GEO_MAPS(chain_chomp_geo);
    FILL_GEO_MAPS(haunted_chair_geo);
    FILL_GEO_MAPS(checkerboard_platform_geo);
    FILL_GEO_MAPS(chilly_chief_geo);
    FILL_GEO_MAPS(chilly_chief_big_geo);
    FILL_GEO_MAPS(chuckya_geo);
    FILL_GEO_MAPS(clam_shell_geo);
    FILL_GEO_MAPS(yellow_coin_geo);
    FILL_GEO_MAPS(yellow_coin_no_shadow_geo);
    FILL_GEO_MAPS(blue_coin_geo);
    FILL_GEO_MAPS(blue_coin_no_shadow_geo);
    FILL_GEO_MAPS(red_coin_geo);
    FILL_GEO_MAPS(red_coin_no_shadow_geo);
    FILL_GEO_MAPS(cyan_fish_geo);
    FILL_GEO_MAPS(dirt_animation_geo);
    FILL_GEO_MAPS(cartoon_star_geo);
    FILL_GEO_MAPS(castle_door_geo);
    FILL_GEO_MAPS(cabin_door_geo);
    FILL_GEO_MAPS(wooden_door_geo);
    FILL_GEO_MAPS(wooden_door2_geo);
    FILL_GEO_MAPS(metal_door_geo);
    FILL_GEO_MAPS(hazy_maze_door_geo);
    FILL_GEO_MAPS(haunted_door_geo);
    FILL_GEO_MAPS(castle_door_0_star_geo);
    FILL_GEO_MAPS(castle_door_1_star_geo);
    FILL_GEO_MAPS(castle_door_3_stars_geo);
    FILL_GEO_MAPS(key_door_geo);
    FILL_GEO_MAPS(dorrie_geo);
    FILL_GEO_MAPS(exclamation_box_geo);
    FILL_GEO_MAPS(exclamation_box_outline_geo);
    FILL_GEO_MAPS(explosion_geo);
    FILL_GEO_MAPS(eyerok_left_hand_geo);
    FILL_GEO_MAPS(eyerok_right_hand_geo);
    FILL_GEO_MAPS(red_flame_shadow_geo);
    FILL_GEO_MAPS(red_flame_geo);
    FILL_GEO_MAPS(blue_flame_geo);
    FILL_GEO_MAPS(flyguy_geo);
    FILL_GEO_MAPS(fwoosh_geo);
    FILL_GEO_MAPS(goomba_geo);
    FILL_GEO_MAPS(haunted_cage_geo);
    FILL_GEO_MAPS(heart_geo);
    FILL_GEO_MAPS(heave_ho_geo);
    FILL_GEO_MAPS(hoot_geo);
    FILL_GEO_MAPS(invisible_bowser_accessory_geo);
    FILL_GEO_MAPS(bowser_impact_smoke_geo);
    FILL_GEO_MAPS(king_bobomb_geo);
    FILL_GEO_MAPS(klepto_geo);
    FILL_GEO_MAPS(koopa_without_shell_geo);
    FILL_GEO_MAPS(koopa_with_shell_geo);
    FILL_GEO_MAPS(koopa_flag_geo);
    FILL_GEO_MAPS(koopa_shell_geo);
    FILL_GEO_MAPS(koopa_shell2_geo);
    FILL_GEO_MAPS(koopa_shell3_geo);
    FILL_GEO_MAPS(lakitu_geo);
    FILL_GEO_MAPS(enemy_lakitu_geo);
    FILL_GEO_MAPS(leaves_geo);
    FILL_GEO_MAPS(mad_piano_geo);
    FILL_GEO_MAPS(manta_seg5_geo_05008D14);
    FILL_GEO_MAPS(mario_geo);
    FILL_GEO_MAPS(marios_cap_geo);
    FILL_GEO_MAPS(marios_metal_cap_geo);
    FILL_GEO_MAPS(marios_wing_cap_geo);
    FILL_GEO_MAPS(marios_winged_metal_cap_geo);
    FILL_GEO_MAPS(metal_box_geo);
    FILL_GEO_MAPS(mips_geo);
    FILL_GEO_MAPS(mist_geo);
    FILL_GEO_MAPS(white_puff_geo);
    FILL_GEO_MAPS(moneybag_geo);
    FILL_GEO_MAPS(monty_mole_geo);
    FILL_GEO_MAPS(mr_i_geo);
    FILL_GEO_MAPS(mr_i_iris_geo);
    FILL_GEO_MAPS(mushroom_1up_geo);
    FILL_GEO_MAPS(number_geo);
    FILL_GEO_MAPS(peach_geo);
    FILL_GEO_MAPS(penguin_geo);
    FILL_GEO_MAPS(piranha_plant_geo);
    FILL_GEO_MAPS(pokey_head_geo);
    FILL_GEO_MAPS(pokey_body_part_geo);
    FILL_GEO_MAPS(wooden_post_geo);
    FILL_GEO_MAPS(purple_switch_geo);
    FILL_GEO_MAPS(scuttlebug_geo);
    FILL_GEO_MAPS(seaweed_geo);
    FILL_GEO_MAPS(skeeter_geo);
    FILL_GEO_MAPS(small_key_geo);
    FILL_GEO_MAPS(mr_blizzard_hidden_geo);
    FILL_GEO_MAPS(mr_blizzard_geo);
    FILL_GEO_MAPS(snufit_geo);
    FILL_GEO_MAPS(sparkles_geo);
    FILL_GEO_MAPS(sparkles_animation_geo);
    FILL_GEO_MAPS(spindrift_geo);
    FILL_GEO_MAPS(spiny_geo);
    FILL_GEO_MAPS(spiny_ball_geo);
    FILL_GEO_MAPS(springboard_top_geo);
    FILL_GEO_MAPS(springboard_spring_geo);
    FILL_GEO_MAPS(springboard_bottom_geo);
    FILL_GEO_MAPS(star_geo);
    FILL_GEO_MAPS(small_water_splash_geo);
    FILL_GEO_MAPS(mario_TODO_geo_0000E0);
    FILL_GEO_MAPS(sushi_geo);
    FILL_GEO_MAPS(swoop_geo);
    FILL_GEO_MAPS(thwomp_geo);
    FILL_GEO_MAPS(toad_geo);
    FILL_GEO_MAPS(tweester_geo);
    FILL_GEO_MAPS(transparent_star_geo);
    FILL_GEO_MAPS(treasure_chest_base_geo);
    FILL_GEO_MAPS(treasure_chest_lid_geo);
    FILL_GEO_MAPS(bubbly_tree_geo);
    FILL_GEO_MAPS(spiky_tree_geo);
    FILL_GEO_MAPS(snow_tree_geo);
    FILL_GEO_MAPS(spiky_tree1_geo);
    FILL_GEO_MAPS(palm_tree_geo);
    FILL_GEO_MAPS(ukiki_geo);
    FILL_GEO_MAPS(unagi_geo);
    FILL_GEO_MAPS(smoke_geo);
    FILL_GEO_MAPS(warp_pipe_geo);
    FILL_GEO_MAPS(water_bomb_geo);
    FILL_GEO_MAPS(water_bomb_shadow_geo);
    FILL_GEO_MAPS(water_mine_geo);
    FILL_GEO_MAPS(water_ring_geo);
    FILL_GEO_MAPS(water_splash_geo);
    FILL_GEO_MAPS(idle_water_wave_geo);
    FILL_GEO_MAPS(wave_trail_geo);
    FILL_GEO_MAPS(white_particle_geo);
    FILL_GEO_MAPS(whomp_geo);
    FILL_GEO_MAPS(wiggler_body_geo);
    FILL_GEO_MAPS(wiggler_head_geo);
    FILL_GEO_MAPS(wooden_signpost_geo);
    FILL_GEO_MAPS(bowser_1_yellow_sphere_geo);
    FILL_GEO_MAPS(yellow_sphere_geo);
    FILL_GEO_MAPS(yoshi_geo);
    FILL_GEO_MAPS(yoshi_egg_geo);
}

#endif