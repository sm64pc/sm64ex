#include "saturn.h"
#include "moon/mod-engine/hooks/hook.h"

#include "moon/utils/moon-env.h"
#include "moon/fs/moonfs.h"
#include "moon/ui/screens/addons/addons-view.h"
#include "moon/mod-engine/engine.h"
#include "moon/imgui/imgui_impl.h"
#include "pc/configfile.h"
#include "pc/controller/controller_keyboard.h"

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
#include "game/behavior_actions.h"
#include "game/behaviors/yoshi.inc.h"
}

bool camera_frozen;
bool enable_head_rotations;
bool enable_shadows;
bool enable_god;
bool enable_dust_particles;

bool show_menu_bar;

float camera_speed = 0.8f;
bool enable_night_skybox;
bool enable_yoshi;

// Second Check

bool has_changed_night_skybox;
bool has_changed_yoshi;

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
                enable_god = false;
                enable_yoshi = false;

                MoonInternal::load_cc_directory();
                MoonInternal::load_cc_file(cc_array[configColorCode]);
                apply_editor_from_cc();
                
                // custom textures
                current_eye_state = 0;
                saturn_load_eye_array();
                custom_eye_name = "saturn/eyes/" + eye_array[0];
                saturn_eye_swap();

                saturn_load_emblem_array();
                custom_emblem_name = "saturn/blank";
                saturn_emblem_swap();

                saturn_load_stache_array();
                saturn_load_button_array();
                saturn_load_sideburn_array();

                // addons
                if(texturePackList.empty()){
                    texturePackList.clear();
                    copy(Moon::addons.begin(), Moon::addons.end(), back_inserter(texturePackList));
                    reverse(texturePackList.begin(), texturePackList.end());
                }
                currentPack = NULL;
            }});

            Moon::registerHookListener({.hookName = WINDOW_API_HANDLE_EVENTS, .callback = [&](HookCall call){
                SDL_Event* ev = (SDL_Event*) call.baseArgs["event"];
                switch (ev->type){
                    case SDL_KEYDOWN:
                        if(ev->key.keysym.sym == SDLK_f){
                            if (accept_input)
                                freeze_camera();
                        }
                        if(ev->key.keysym.sym == SDLK_F1){
                            show_menu_bar = !show_menu_bar;
                        }
                    case SDL_CONTROLLERBUTTONDOWN:
                        if (ev->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP) {
                            freeze_camera();
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

                // Yoshi

                enableYoshi = (enable_yoshi) ? 1 : 0;
            }});
        }
    }
}