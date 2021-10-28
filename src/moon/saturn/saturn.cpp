#include "saturn.h"
#include "moon/mod-engine/hooks/hook.h"

#include "moon/utils/moon-env.h"
#include "moon/fs/moonfs.h"
#include "pc/configfile.h"

#include "saturn_colors.h"
#include "saturn_textures.h"

#include <SDL2/SDL.h>

#include <iostream>
#include <string>
#include <vector>
using namespace std;
#include <dirent.h>
#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;

extern "C" {
#include "game/camera.h"
#include "game/level_update.h"
#include "game/mario.h"
#include "sm64.h"
}

bool camera_frozen;
bool enable_head_rotations;
bool enable_shadows;
bool enable_dust_particles;

bool show_menu_bar;

float camera_speed = 0.8f;
bool enable_cap_logo;
bool enable_overall_buttons;

// Second Check

bool has_changed_cap_logo;
bool has_changed_overall_buttons;

namespace MoonInternal {

    // Machinima

    void freeze_camera() {
        camera_frozen = !camera_frozen;
        //camVelSpeed = 1.0f;
    }
    void cycle_eye_state(int cycle) {
        current_eye_state += cycle;
    }

    // Setup Module

    void setupSaturnModule(string status){
        if(status == "PreStartup"){

            Moon::registerHookListener({.hookName = WINDOW_API_INIT, .callback = [&](HookCall call){
                show_menu_bar = false;

                camera_frozen = false;
                enable_shadows = true;
                enable_cap_logo = true;
                enable_overall_buttons = true;

                MoonInternal::load_cc_directory();
                
                // custom textures
                current_eye_state = 0;
                saturn_load_eye_array();
                custom_eye_name = "eyes/" + eye_array[0];
                saturn_eye_swap();
                saturn_toggle_m_cap();
                saturn_toggle_m_buttons();
            }});

            Moon::registerHookListener({.hookName = WINDOW_API_HANDLE_EVENTS, .callback = [&](HookCall call){
                SDL_Event* ev = (SDL_Event*) call.baseArgs["event"];
                switch (ev->type){
                    case SDL_KEYDOWN:
                        if(ev->key.keysym.sym == SDLK_f){
                            freeze_camera();
                        }
                        if(ev->key.keysym.sym == SDLK_x){
                            //cycle_eye_state(1);
                        }
                        if(ev->key.keysym.sym == SDLK_z){
                            //cycle_eye_state(-1);
                        }
                        if(ev->key.keysym.sym == SDLK_F1){
                            show_menu_bar = !show_menu_bar;
                        }
                    case SDL_CONTROLLERBUTTONDOWN:
                        if (ev->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP) {
                            freeze_camera();
                        }
                        if (ev->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) {
                            //cycle_eye_state(1);
                        }
                        if (ev->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT) {
                            //cycle_eye_state(-1);
                        }
                        if(ev->cbutton.button == SDL_CONTROLLER_BUTTON_BACK){
                            show_menu_bar = !show_menu_bar;
                        }
                    break;
                }
            }});

            Moon::registerHookListener({.hookName = GFX_PRE_START_FRAME, .callback = [&](HookCall call){
                // Machinima Camera

                machinimaMode = (camera_frozen) ? 1 : 0;
                camVelSpeed = camera_speed;

                /* OLD CAMERA...
                
                if (camera_frozen == true) {
                    if (set_cam_angle(0) != CAM_ANGLE_MARIO) {
                        gLakituState.focVSpeed = camera_speed;
                        gLakituState.focHSpeed = camera_speed;
                    }
                    gLakituState.posVSpeed = camera_speed;
                    gLakituState.posHSpeed = camera_speed;
                    gCamera->nextYaw = calculate_yaw(gLakituState.focus, gLakituState.pos);
                    gCamera->yaw = gCamera->nextYaw;
                    gCameraMovementFlags &= ~CAM_MOVE_FIX_IN_PLACE;
                }
                */

                // Custom Textures

                if (enable_cap_logo && !has_changed_cap_logo) {
                    saturn_toggle_m_cap();
                    has_changed_cap_logo = true;
                }
                if (!enable_overall_buttons && has_changed_overall_buttons) {
                    saturn_toggle_m_buttons();
                    has_changed_overall_buttons = false;
                }
            }});
        }
    }
}