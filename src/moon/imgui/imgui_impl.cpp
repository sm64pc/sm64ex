#include "imgui_impl.h"

#include <map>
#include <string>
#include <iostream>

#include "moon/libs/imgui/imgui.h"
#include "moon/libs/imgui/imgui_internal.h"
#include "moon/libs/imgui/imgui_impl_sdl.h"
#include "moon/libs/imgui/imgui_impl_opengl3.h"
#include "moon/libs/imgui/imgui_switch_impl.h"
#include "moon/mod-engine/hooks/hook.h"
#include "moon/mod-engine/textures/mod-texture.h"
#include "moon/mod-engine/engine.h"
#include "moon/texts/moon-loader.h"
#include "moon/saturn/saturn.h"
#include "moon/saturn/saturn_colors.h"
#include "moon/saturn/saturn_textures.h"
#include "moon/saturn/saturn_types.h"
#include "icons/IconsForkAwesome.h"
#include "icons/IconsMaterialDesign.h"
#include "moon/utils/moon-env.h"

#include <SDL2/SDL.h>

#ifdef __MINGW32__
# define FOR_WINDOWS 1
#else
# define FOR_WINDOWS 0
#endif

#if FOR_WINDOWS || defined(OSX_BUILD)
# define GLEW_STATIC
# include <GL/glew.h>
#endif

#ifdef TARGET_SWITCH
#include "glad/glad.h"
#include <switch.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/errno.h>
extern "C" {
#include "nx/m_nx.h"
}
# define RAPI_NAME "OpenGL 4.2"
#else
#define GL_GLEXT_PROTOTYPES 1
#ifdef USE_GLES
# include <SDL2/SDL_opengles2.h>
# define RAPI_NAME "OpenGL ES"
#else
# include <SDL2/SDL_opengl.h>
# define RAPI_NAME "OpenGL"
#endif
#endif

#if FOR_WINDOWS
#define PLATFORM "Windows"
#elif defined(OSX_BUILD)
#define PLATFORM "MacOS"
#elif defined(TARGET_SWITCH)
#define PLATFORM "Nintendo Switch"
#else
#define PLATFORM "Linux"
#endif

extern "C" {
#include "pc/gfx/gfx_pc.h"
#include "pc/pc_main.h"
#include "game/camera.h"
#include "game/mario.h"
#include "game/level_update.h"
#include "engine/level_script.h"
#include "level_table.h"
}

#include "pc/configfile.h"

#define DEFAULT_FONT "monogram.ttf"

using namespace std;

SDL_Window* window = nullptr;
ImGuiIO* io = nullptr;

#define SM64_WIDTH  960
#define SM64_HEIGHT 720

// Helper to display a little (?) mark which shows a tooltip when hovered.
static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void warp_to(s16 destLevel, s16 destArea = 0x01, s16 destWarpNode = 0x0A) {
    if (gCurrLevelNum == destLevel || !mario_loaded) {
        return;
    }

    mario_loaded = false;
    initiate_warp(destLevel, destArea, destWarpNode, 0);
    fade_into_special_warp(0,0);
}

#ifdef TARGET_SWITCH
namespace MoonNX {
    SwkbdConfig kbd;

    static int waitFramesToUpdate = 0;

    void handleVirtualKeyboard(string status){
        if(status == "Init"){
            Result rc = 0;
            char tmpoutstr[16] = {0};
            rc = swkbdCreate(&kbd, 0);
            if (R_SUCCEEDED(rc))
                swkbdConfigMakePresetDefault(&kbd);
        }

        if(status == "FrameUpdate") {
            ImGuiIO* io = &ImGui::GetIO();
            int length = 512;
            char* message;

            if(waitFramesToUpdate > 0)
                waitFramesToUpdate--;

            if(waitFramesToUpdate){
                ImGui::ClearActiveID();
                free(message);
            }

            if(io->WantTextInput && !waitFramesToUpdate){
                message = (char*)malloc(length);
                ImGuiInputTextState* state = ImGui::GetInputTextState(ImGui::GetActiveID());
                if(!state->InitialTextA.empty()){
                    swkbdConfigSetInitialText(&kbd, state->InitialTextA.Data);
                }

                Result rc = swkbdShow(&kbd, message, length);

                if(R_SUCCEEDED(rc)){
                    state->ClearText();
                    state->OverwriteData = &message[0];
                }

                waitFramesToUpdate = 2;
                io->WantTextInput = false;
            }
        }
    }
}
#endif

int n64Mul = 1;

namespace MoonInternal {

    map<string, ImFont*> fontMap;

    // Colors
    static ImVec4 uiHatColor =              ImVec4(255.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f);
    static ImVec4 uiHatShadeColor =         ImVec4(127.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f);
    static ImVec4 uiOverallsColor =         ImVec4(0.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f);
    static ImVec4 uiOverallsShadeColor =    ImVec4(0.0f / 255.0f, 0.0f / 255.0f, 127.0f / 255.0f, 255.0f / 255.0f);
    static ImVec4 uiGlovesColor =           ImVec4(255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f);
    static ImVec4 uiGlovesShadeColor =      ImVec4(127.0f / 255.0f, 127.0f / 255.0f, 127.0f / 255.0f, 255.0f / 255.0f);
    static ImVec4 uiShoesColor =            ImVec4(114.0f / 255.0f, 28.0f / 255.0f, 14.0f / 255.0f, 255.0f / 255.0f);
    static ImVec4 uiShoesShadeColor =       ImVec4(57.0f / 255.0f, 14.0f / 255.0f, 7.0f / 255.0f, 255.0f / 255.0f);
    static ImVec4 uiSkinColor =             ImVec4(254.0f / 255.0f, 193.0f / 255.0f, 121.0f / 255.0f, 255.0f / 255.0f);
    static ImVec4 uiSkinShadeColor =        ImVec4(127.0f / 255.0f, 96.0f / 255.0f, 60.0f / 255.0f, 255.0f / 255.0f);
    static ImVec4 uiHairColor =             ImVec4(115.0f / 255.0f, 6.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f);
    static ImVec4 uiHairShadeColor =        ImVec4(57.0f / 255.0f, 3.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f);

    static char bufname[128] = "Sample";

    bool hasChangedFullscreen;
    int tempXRes;
    int tempYRes;

    int selected_eye_item = 0;

    int selected_emblem_item = 1;
    int current_emblem_item = 1;
    int selected_stache_item = 0;
    int current_stache_item = 0;
    int selected_button_item = 0;
    int current_button_item = 0;
    int selected_sideburn_item = 0;
    int current_sideburn_item = 0;

    int selected_sky_item = 0;
    int current_sky_item = 0;

    int current_level_sel = 0;

    void setupFonts() {
        ImGuiIO& io = ImGui::GetIO();
        // for (auto entry = Moon::fonts.begin(); entry != Moon::fonts.end(); entry++){
        //     if(entry->first == FONT_ICON_FILE_NAME_FK) continue;
        //     ImFont* tmp = io.Fonts->AddFontFromMemoryTTF((void*) entry->second->data, entry->second->size, 18.f, &font_cfg);
        //     cout << "Loading font: " << entry->first << endl;
        //     fontMap[entry->first] = tmp;
        // }
        // io.FontDefault = fontMap[DEFAULT_FONT];

        // Setup Material Design Icons
        ImFontConfig font_cfg;
        font_cfg.GlyphOffset = ImVec2(0.0f, -2.0f);
        static const ImWchar icons_ranges[] = { ICON_MIN_MD, ICON_MAX_MD, 0 };

        io.FontDefault = io.Fonts->AddFontFromMemoryTTF((void*) Moon::fonts[DEFAULT_FONT]->data, Moon::fonts[DEFAULT_FONT]->size, 18.f, &font_cfg);
        ImFontConfig config;
        config.GlyphOffset = ImVec2(0.0f, 2.0f);
        config.MergeMode = true;
        io.Fonts->AddFontFromMemoryTTF((void*) Moon::fonts[FONT_ICON_FILE_NAME_MD]->data, Moon::fonts[FONT_ICON_FILE_NAME_MD]->size, 20.f, &config, icons_ranges);
        io.Fonts->Build();
    }

    void apply_cc_from_editor() {
        defaultColorHatRLight = (int)(uiHatColor.x * 255);
        defaultColorHatGLight = (int)(uiHatColor.y * 255);
        defaultColorHatBLight = (int)(uiHatColor.z * 255);
        defaultColorHatRDark = (int)(uiHatShadeColor.x * 255);
        defaultColorHatGDark = (int)(uiHatShadeColor.y * 255);
        defaultColorHatBDark = (int)(uiHatShadeColor.z * 255);
        defaultColorOverallsRLight = (int)(uiOverallsColor.x * 255);
        defaultColorOverallsGLight = (int)(uiOverallsColor.y * 255);
        defaultColorOverallsBLight = (int)(uiOverallsColor.z * 255);
        defaultColorOverallsRDark = (int)(uiOverallsShadeColor.x * 255);
        defaultColorOverallsGDark = (int)(uiOverallsShadeColor.y * 255);
        defaultColorOverallsBDark = (int)(uiOverallsShadeColor.z * 255);
        defaultColorGlovesRLight = (int)(uiGlovesColor.x * 255);
        defaultColorGlovesGLight = (int)(uiGlovesColor.y * 255);
        defaultColorGlovesBLight = (int)(uiGlovesColor.z * 255);
        defaultColorGlovesRDark = (int)(uiGlovesShadeColor.x * 255);
        defaultColorGlovesGDark = (int)(uiGlovesShadeColor.y * 255);
        defaultColorGlovesBDark = (int)(uiGlovesShadeColor.z * 255);
        defaultColorShoesRLight = (int)(uiShoesColor.x * 255);
        defaultColorShoesGLight = (int)(uiShoesColor.y * 255);
        defaultColorShoesBLight = (int)(uiShoesColor.z * 255);
        defaultColorShoesRDark = (int)(uiShoesShadeColor.x * 255);
        defaultColorShoesGDark = (int)(uiShoesShadeColor.y * 255);
        defaultColorShoesBDark = (int)(uiShoesShadeColor.z * 255);
        defaultColorSkinRLight = (int)(uiSkinColor.x * 255);
        defaultColorSkinGLight = (int)(uiSkinColor.y * 255);
        defaultColorSkinBLight = (int)(uiSkinColor.z * 255);
        defaultColorSkinRDark = (int)(uiSkinShadeColor.x * 255);
        defaultColorSkinGDark = (int)(uiSkinShadeColor.y * 255);
        defaultColorSkinBDark = (int)(uiSkinShadeColor.z * 255);
        defaultColorHairRLight = (int)(uiHairColor.x * 255);
        defaultColorHairGLight = (int)(uiHairColor.y * 255);
        defaultColorHairBLight = (int)(uiHairColor.z * 255);
        defaultColorHairRDark = (int)(uiHairShadeColor.x * 255);
        defaultColorHairGDark = (int)(uiHairShadeColor.y * 255);
        defaultColorHairBDark = (int)(uiHairShadeColor.z * 255);
    }

    void setupImGuiModule(string status) {
        MoonInternal::setupWindowHook(status);
        if(status == "PreStartup"){
            Moon::registerHookListener({.hookName = WINDOW_API_INIT, .callback = [](HookCall call){
                const char* glsl_version = "#version 120";
                ImGuiContext* ctx = ImGui::CreateContext();
                ImGui::SetCurrentContext(ctx);
                io = &ImGui::GetIO();
                io->WantSetMousePos = false;
                io->ConfigWindowsMoveFromTitleBarOnly = true;
                io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

                ImGui::StyleColorsMoonDark();

                setupFonts();

                MoonInternal::bindHook(IMGUI_API_INIT);
                MoonInternal::initBindHook(1,
                    (struct HookParameter){.name = "io", .parameter = (void*) &io}
                );
                MoonInternal::callBindHook(0);

                window = (SDL_Window*) call.baseArgs["window"];
                ImGui_ImplSDL2_InitForOpenGL(window, call.baseArgs["context"]);
                ImGui_ImplOpenGL3_Init(glsl_version);

                tempXRes = configWindow.w;
                tempYRes = configWindow.h;

            #ifdef TARGET_SWITCH
                MoonNX::handleVirtualKeyboard("Init");
            #endif

                static char cc_gameshark[1024 * 16] = "";
                apply_cc_from_editor();
                strcpy(cc_gameshark, global_color_to_cc().c_str());
            }});

            Moon::registerHookListener({.hookName = WINDOW_API_HANDLE_EVENTS, .callback = [](HookCall call){
                SDL_Event* ev = (SDL_Event*) call.baseArgs["event"];
                ImGui_ImplSDL2_ProcessEvent(ev);
            }});


            Moon::registerHookListener({ GFX_POST_END_FRAME, [](HookCall call){
                // recv(socketID, NULL, 1, MSG_PEEK | MSG_DONTWAIT) != 0
                // bool retval = 0;
                io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplSDL2_NewFrame(window);
                ImGui::NewFrame();
            #ifdef TARGET_SWITCH
                MoonNX::handleVirtualKeyboard("FrameUpdate");
            #endif
                MoonInternal::bindHook(IMGUI_API_DRAW);
                MoonInternal::initBindHook(0);
                MoonInternal::callBindHook(0);

                ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_NoWindowMenuButton;

                // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
                // because it would be confusing to have two docking targets within each others.
                ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
                const ImGuiViewport* viewport = ImGui::GetMainViewport();
                ImGui::SetNextWindowPos(viewport->WorkPos);
                ImGui::SetNextWindowSize(ImVec2(configWindow.w, configWindow.h));
                ImGui::SetNextWindowViewport(viewport->ID);
                window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;
                window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoResize;

                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
                ImGui::Begin("Main - Deck", nullptr, window_flags);
                ImGui::PopStyleVar();
                ImGuiID dockspace_id = ImGui::GetID("main_dock");

                if (!ImGui::DockBuilderGetNode(dockspace_id)) {
                    ImGui::DockBuilderRemoveNode(dockspace_id);
                    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_NoTabBar);
                    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_NoDockingOverMe);

                    ImGui::DockBuilderDockWindow("Game", dockspace_id);

                    ImGui::DockBuilderFinish(dockspace_id);
                }

                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

                //if (show_menu_bar) {
                    if (ImGui::BeginMenuBar()) {
                        /*
                        TextureData* tmp = forceTextureLoad("mod-icons://Moon64");
                        if(tmp != nullptr) {
                            ImGui::SetCursorPos(ImVec2(0, 0));
                            ImGui::Image((ImTextureID) tmp->texture_id, ImVec2(25.0f, 25.0f));
                            ImGui::SameLine();
                        }
                        */
                        ImGui::Text("Saturn");
                        ImGui::SameLine();
                        ImGui::Separator();
                        if (ImGui::BeginMenu("Tools")) {
                            ImGui::MenuItem("Toggle View (F1)", NULL, &show_menu_bar);
                            ImGui::MenuItem("Jabo Mode", NULL, &configImGui.jaboMode);
                            //ImGui::MenuItem("Night Mode", NULL, &enable_night_skybox);
                            ImGui::EndMenu();
                        }
                        if (ImGui::BeginMenu("View")) {
                            ImGui::MenuItem("Stats", NULL, &configImGui.s_stats);
                            ImGui::MenuItem("Machinima", NULL, &configImGui.s_machinima);
                            ImGui::MenuItem("Quick Options", NULL, &configImGui.s_toggles);
                            ImGui::MenuItem("Appearance", NULL, &configImGui.s_appearance);
                            //ImGui::MenuItem("Debug Textures", NULL, &configImGui.texture_debug);
                            ImGui::EndMenu();
                        }
                        if (ImGui::BeginMenu("Misc")) {
                            ImGui::MenuItem("Settings", NULL, &configImGui.s_options);
                            ImGui::EndMenu();
                        }
                        ImGui::EndMenuBar();
                    }
                //}
                ImGui::End();
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f)); // Set window background to red
                ImGui::Begin("Game", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();

                ImVec2 size = ImGui::GetContentRegionAvail();
                ImVec2 pos = ImVec2(0, 0);

                if (configWindow.fullscreen) {
                    if (!hasChangedFullscreen) {
                        std::cout << "fullscreen test" << std::endl;
                        tempXRes = configWindow.w;
                        tempYRes = configWindow.h;
                        SDL_DisplayMode DM;
                        SDL_GetCurrentDisplayMode(0, &DM);
                        configWindow.w = DM.w;
                        configWindow.h = DM.h;
                        hasChangedFullscreen = true;
                    }
                } else {
                    if (hasChangedFullscreen) {
                        std::cout << "test2 fsd" << std::endl;
                        configWindow.w = tempXRes;
                        configWindow.h = tempYRes;
                        SDL_SetWindowSize(window, tempXRes, tempYRes);
                        hasChangedFullscreen = false;
                    }
                }

                configWindow.internal_w = (configImGui.jaboMode && !show_menu_bar) ? SM64_WIDTH : size.x;
                configWindow.internal_h = (configImGui.jaboMode && !show_menu_bar) ? SM64_HEIGHT : size.y;

                /*
                if(configImGui.jaboMode) {
                    configWindow.multiplier = (float)n64Mul;
                    int sw = size.y * SM64_WIDTH / SM64_HEIGHT;
                    pos = ImVec2(size.x / 2 - sw / 2, 0);
                    size = ImVec2(sw,  size.y);
                }
                */

                int fbuf = stoi(MoonInternal::getEnvironmentVar("framebuffer"));
                ImGui::ImageRotated((ImTextureID) fbuf, pos, size, 180.0f);
                ImGui::End();

                if (configImGui.s_stats && show_menu_bar){
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
                    ImGui::Begin("Stats", NULL, ImGuiWindowFlags_None);

                    ImGui::Text("Platform: " PLATFORM " (" RAPI_NAME ")");
                    ImGui::Text("View: %.0fx%.0f (%.1f FPS)", configWindow.internal_w * configWindow.multiplier, configWindow.internal_h * configWindow.multiplier, ImGui::GetIO().Framerate);
                    ImGui::Text("Version: " GIT_BRANCH " " GIT_HASH);

                    ImGui::End();
                    ImGui::PopStyleColor();
                }
                if (configImGui.s_toggles && show_menu_bar){
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
                    ImGui::Begin("Quick Options", NULL, ImGuiWindowFlags_None);

                    ImGui::Dummy(ImVec2(0, 5));

                    ImGui::Checkbox("HUD", &configHUD);
                    ImGui::Checkbox("Head Rotations", &enable_head_rotations);

                    const char* capStates[] = { "Cap On", "Cap Off", "Wing Cap" }; // unused "wing cap off" not included
                    ImGui::Combo("Cap", &current_cap_state, capStates, IM_ARRAYSIZE(capStates));
                    const char* handStates[] = { "Fists", "Open", "Peace", "With Cap", "With Wing Cap", "Right Open" };
                    ImGui::Combo("Hands", &current_hand_state, handStates, IM_ARRAYSIZE(handStates));

                    ImGui::Dummy(ImVec2(0, 5));
                    
                    ImGui::Checkbox("Dust Particles", &enable_dust_particles);
                    ImGui::Checkbox("Shadows", &enable_shadows);

                    ImGui::Dummy(ImVec2(0, 5));

                    ImGui::Checkbox("God Mode", &enable_god);

                    ImGui::Dummy(ImVec2(0, 5));

                    const char* levelList[] = { 
                        "Castle Grounds", "Castle Inside", "Bob-omb Battlefield", 
                        "Whomp's Fortress", "Princess's Secret Slide", "Tower of the Wing Cap", 
                        "Jolly Roger Bay", "Secret Aquarium", "Cool, Cool Mountain", 
                        "Bowser in the Dark World", "Big Boo's Haunt", "Hazy Maze Cave", 
                        "Cavern of the Metal Cap", "Lethal Lava Land", "Shifting Sand Land", 
                        "Vanish Cap under the Moat", "Dire, Dire Docks", "Bowser in the Fire Sea", 
                        "Snowman's Land", "Wet-Dry World", "Tall, Tall Mountain", "Tiny, Huge Island",
                        "Tick Tock Clock", "Wing Mario Over the Rainbow", "Rainbow Ride", "Bowser in the Sky"
                    };
                    ImGui::Combo("Level", &current_level_sel, levelList, IM_ARRAYSIZE(levelList));

                    if (ImGui::Button("Warp")) {
                        switch (current_level_sel) {
                            case 0:
                                warp_to(LEVEL_CASTLE_GROUNDS, 0x01, 0x04);
                                break;
                            case 1:
                                warp_to(LEVEL_CASTLE, 0x01, 0x01);
                                break;
                            case 2:
                                warp_to(LEVEL_BOB);
                                break;
                            case 3:
                                warp_to(LEVEL_WF);
                                break;
                            case 4:
                                warp_to(LEVEL_PSS);
                                break;
                            case 5:
                                warp_to(LEVEL_TOTWC);
                                break;
                            case 6:
                                warp_to(LEVEL_JRB);
                                break;
                            case 7:
                                warp_to(LEVEL_SA);
                                break;
                            case 8:
                                warp_to(LEVEL_CCM);
                                break;
                            case 9:
                                warp_to(LEVEL_BITDW);
                                break;
                            case 10:
                                warp_to(LEVEL_BBH);
                                break;
                            case 11:
                                warp_to(LEVEL_HMC);
                                break;
                            case 12:
                                warp_to(LEVEL_COTMC);
                                break;
                            case 13:
                                warp_to(LEVEL_LLL);
                                break;
                            case 14:
                                warp_to(LEVEL_SSL);
                                break;
                            case 15:
                                warp_to(LEVEL_VCUTM);
                                break;
                            case 16:
                                warp_to(LEVEL_DDD);
                                break;
                            case 17:
                                warp_to(LEVEL_BITFS);
                                break;
                            case 18:
                                warp_to(LEVEL_SL);
                                break;
                            case 19:
                                warp_to(LEVEL_WDW);
                                break;
                            case 20:
                                warp_to(LEVEL_TTM);
                                break;
                            case 21:
                                warp_to(LEVEL_THI);
                                break;
                            case 22:
                                warp_to(LEVEL_TTC);
                                break;
                            case 23:
                                warp_to(LEVEL_WMOTR);
                                break;
                            case 24:
                                warp_to(LEVEL_RR);
                                break;
                            case 25:
                                warp_to(LEVEL_BITS);
                                break;
                            default:
                                warp_to(LEVEL_CASTLE_GROUNDS);
                                break;
                        }
                    }

                    ImGui::Dummy(ImVec2(0, 5));

                    /*
                    const char* skyStates[] = { "Default", "Night", "Green", "Blue", "Pink"};
                    ImGui::Combo("Skybox", &selected_sky_item, skyStates, IM_ARRAYSIZE(skyStates));

                    if (selected_sky_item != current_sky_item) {
                        current_sky_item = selected_sky_item;
                        if (selected_sky_item == 0) custom_sky_name = "default";
                        if (selected_sky_item == 1) custom_sky_name = "saturn/skyboxes/night";
                        if (selected_sky_item == 2) custom_sky_name = "saturn/skyboxes/green";
                        if (selected_sky_item == 3) custom_sky_name = "saturn/skyboxes/blue";
                        if (selected_sky_item == 4) custom_sky_name = "saturn/skyboxes/pink";
                        saturn_sky_swap();
                    }
                    */

                    ImGui::End();
                    ImGui::PopStyleColor();
                }

                static char cc_gameshark[1024 * 16] = "";

                if (configImGui.s_machinima && show_menu_bar) {
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
                    ImGui::Begin("Machinima", NULL, ImGuiWindowFlags_None);

                    ImGui::Checkbox("Machinima Camera", &camera_frozen);
                    if (camera_frozen == true) {
                        ImGui::SliderFloat("Speed", &camera_speed, 0.2f, 0.8f);
                    }

                    ImGui::Dummy(ImVec2(0, 10));

                    ImGui::Text("Select Color Code");
                    static int current_cc_id = 0;
                    string cc_name = MoonInternal::cc_array[current_cc_id].substr(0, MoonInternal::cc_array[current_cc_id].size() - 3);
                    if (ImGui::BeginCombo(".gs", cc_name.c_str()))
                    {
                        for (int n = 0; n < MoonInternal::cc_array.size(); n++)
                        {
                            const bool is_selected = (current_cc_id == n);
                            cc_name = MoonInternal::cc_array[n].substr(0, MoonInternal::cc_array[n].size() - 3);
                            if (ImGui::Selectable(cc_name.c_str(), is_selected)) {
                                current_cc_id = n;
                            }

                            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                            if (is_selected)
                                ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }
                    if (ImGui::Button("Load CC")) {
                        load_cc_file(cc_array[current_cc_id]);

                        uiHatColor = ImVec4(float(defaultColorHatRLight) / 255.0f, float(defaultColorHatGLight) / 255.0f, float(defaultColorHatBLight) / 255.0f, 255.0f / 255.0f);
                        uiHatShadeColor = ImVec4(float(defaultColorHatRDark) / 255.0f, float(defaultColorHatGDark) / 255.0f, float(defaultColorHatBDark) / 255.0f, 255.0f / 255.0f);
                        uiOverallsColor = ImVec4(float(defaultColorOverallsRLight) / 255.0f, float(defaultColorOverallsGLight) / 255.0f, float(defaultColorOverallsBLight) / 255.0f, 255.0f / 255.0f);
                        uiOverallsShadeColor = ImVec4(float(defaultColorOverallsRDark) / 255.0f, float(defaultColorOverallsGDark) / 255.0f, float(defaultColorOverallsBDark) / 255.0f, 255.0f / 255.0f);
                        uiGlovesColor = ImVec4(float(defaultColorGlovesRLight) / 255.0f, float(defaultColorGlovesGLight) / 255.0f, float(defaultColorGlovesBLight) / 255.0f, 255.0f / 255.0f);
                        uiGlovesShadeColor = ImVec4(float(defaultColorGlovesRDark) / 255.0f, float(defaultColorGlovesGDark) / 255.0f, float(defaultColorGlovesBDark) / 255.0f, 255.0f / 255.0f);
                        uiShoesColor = ImVec4(float(defaultColorShoesRLight) / 255.0f, float(defaultColorShoesGLight) / 255.0f, float(defaultColorShoesBLight) / 255.0f, 255.0f / 255.0f);
                        uiShoesShadeColor = ImVec4(float(defaultColorShoesRDark) / 255.0f, float(defaultColorShoesGDark) / 255.0f, float(defaultColorShoesBDark) / 255.0f, 255.0f / 255.0f);
                        uiSkinColor = ImVec4(float(defaultColorSkinRLight) / 255.0f, float(defaultColorSkinGLight) / 255.0f, float(defaultColorSkinBLight) / 255.0f, 255.0f / 255.0f);
                        uiSkinShadeColor = ImVec4(float(defaultColorSkinRDark) / 255.0f, float(defaultColorSkinGDark) / 255.0f, float(defaultColorSkinBDark) / 255.0f, 255.0f / 255.0f);
                        uiHairColor = ImVec4(float(defaultColorHairRLight) / 255.0f, float(defaultColorHairGLight) / 255.0f, float(defaultColorHairBLight) / 255.0f, 255.0f / 255.0f);
                        uiHairShadeColor = ImVec4(float(defaultColorHairRDark) / 255.0f, float(defaultColorHairGDark) / 255.0f, float(defaultColorHairBDark) / 255.0f, 255.0f / 255.0f);
                        
                        // We never want to use the name "Mario" when saving/loading a CC, as it will cause file issues.
                        if (cc_name == "Mario") {
                            strcpy(bufname, "Sample");
                        } else {
                            strcpy(bufname, cc_name.c_str());
                        }

                        strcpy(cc_gameshark, global_color_to_cc().c_str());
                    }

                    ImGui::Dummy(ImVec2(0, 10));

                    ImGui::Text("Eye State");
                    const char* eyeStates[] = { "Default", "Custom...", "Open", "Half", "Closed", "Dead" };
                    ImGui::Combo("Eyes", &selected_eye_item, eyeStates, IM_ARRAYSIZE(eyeStates));
                    if (selected_eye_item == 0) current_eye_state = 0;
                    if (selected_eye_item == 1 && eye_array.size() > 0) {

                        /*
                        SATURN: Moon Edition uses the unused "Left" eye state for custom textures.

                        If you are a model author, you must manually add the switch option to your model.
                        You can do this in Fast64 by navigating to 000-switch.001's bone and adding a material override under Switch Option 3.
                        Switch Option 7 is the "Dead" eye state.
                        */

                        current_eye_state = 4;
                        static int current_eye_id = 0;
                        string eye_name = eye_array[current_eye_id];
                        if (ImGui::BeginCombo(".png ###eye", eye_name.c_str()))
                        {
                            for (int o = 0; o < eye_array.size(); o++)
                            {
                                const bool is_eye_selected = (current_eye_id == o);
                                eye_name = eye_array[o];
                                if (ImGui::Selectable(eye_name.c_str(), is_eye_selected)) {
                                    current_eye_id = o;
                                }

                                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                if (is_eye_selected)
                                    ImGui::SetItemDefaultFocus();
                            }
                            ImGui::EndCombo();
                        }
                        if (ImGui::Button("Set Eyes")) {
                            custom_eye_name = "saturn/eyes/" + eye_array[current_eye_id];
                            saturn_eye_swap();
                        }
                    }
                    if (selected_eye_item == 2) current_eye_state = 1;
                    if (selected_eye_item == 3) current_eye_state = 2;
                    if (selected_eye_item == 4) current_eye_state = 3;
                    if (selected_eye_item == 5) current_eye_state = 8; // dead eyes

                    ImGui::Dummy(ImVec2(0, 5));

                    if (ImGui::CollapsingHeader("Mario")) {
                        const char* emblemStates[] = { "Default", "None", "Custom..." };
                        ImGui::Combo("Emblem", &selected_emblem_item, emblemStates, IM_ARRAYSIZE(emblemStates));
                        if (selected_emblem_item == 0 && current_emblem_item != 0) {
                            current_emblem_item = 0;
                            custom_emblem_name = "default";
                            saturn_emblem_swap();
                        } else if (selected_emblem_item == 1 && current_emblem_item != 1) {
                            current_emblem_item = 1;
                            custom_emblem_name = "saturn/blank";
                            saturn_emblem_swap();
                        } 
                        else if (selected_emblem_item == 2 && emblem_array.size() > 0) {
                            static int current_emblem_id = 0;

                            // Only do this once
                            if (current_emblem_item != 2) {
                                current_emblem_item = 2;
                                custom_emblem_name = "saturn/emblems/" + emblem_array[current_emblem_id];
                                saturn_emblem_swap();
                            }

                            string emblem_name = emblem_array[current_emblem_id];
                            if (ImGui::BeginCombo(".png ###emblem", emblem_name.c_str()))
                            {
                                for (int o = 0; o < emblem_array.size(); o++)
                                {
                                    const bool is_emblem_selected = (current_emblem_id == o);
                                    emblem_name = emblem_array[o];
                                    if (ImGui::Selectable(emblem_name.c_str(), is_emblem_selected)) {
                                        current_emblem_id = o;
                                    }

                                    // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                    if (is_emblem_selected)
                                        ImGui::SetItemDefaultFocus();
                                }
                                ImGui::EndCombo();
                            }
                            if (ImGui::Button("Set Emblem")) {
                                custom_emblem_name = "saturn/emblems/" + emblem_array[current_emblem_id];
                                saturn_emblem_swap();
                            }
                            ImGui::Dummy(ImVec2(0, 5));
                        }

                        const char* stacheStates[] = { "Default", "None", "Custom..." };
                        ImGui::Combo("Stache", &selected_stache_item, stacheStates, IM_ARRAYSIZE(stacheStates));
                        if (selected_stache_item == 0 && current_stache_item != 0) {
                            current_stache_item = 0;
                            custom_stache_name = "default";
                            saturn_stache_swap();
                        } else if (selected_stache_item == 1 && current_stache_item != 1) {
                            current_stache_item = 1;
                            custom_stache_name = "saturn/blank";
                            saturn_stache_swap();
                        } 
                        else if (selected_stache_item == 2 && stache_array.size() > 0) {
                            static int current_stache_id = 0;

                            // Only do this once
                            if (current_stache_item != 2) {
                                current_stache_item = 2;
                                custom_stache_name = "saturn/staches/" + stache_array[current_stache_id];
                                saturn_stache_swap();
                            }

                            string stache_name = stache_array[current_stache_id];
                            if (ImGui::BeginCombo(".png ###stache", stache_name.c_str()))
                            {
                                for (int o = 0; o < stache_array.size(); o++)
                                {
                                    const bool is_stache_selected = (current_stache_id == o);
                                    stache_name = stache_array[o];
                                    if (ImGui::Selectable(stache_name.c_str(), is_stache_selected)) {
                                        current_stache_id = o;
                                    }

                                    // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                    if (is_stache_selected)
                                        ImGui::SetItemDefaultFocus();
                                }
                                ImGui::EndCombo();
                            }
                            if (ImGui::Button("Set Stache")) {
                                custom_stache_name = "saturn/staches/" + stache_array[current_stache_id];
                                saturn_stache_swap();
                            }
                            ImGui::Dummy(ImVec2(0, 5));
                        }

                        const char* buttonStates[] = { "Default", "None", "Custom..." };
                        ImGui::Combo("Buttons", &selected_button_item, buttonStates, IM_ARRAYSIZE(buttonStates));
                        if (selected_button_item == 0 && current_button_item != 0) {
                            current_button_item = 0;
                            custom_button_name = "default";
                            saturn_button_swap();
                        } else if (selected_button_item == 1 && current_button_item != 1) {
                            current_button_item = 1;
                            custom_button_name = "saturn/blank";
                            saturn_button_swap();
                        } 
                        else if (selected_button_item == 2 && button_array.size() > 0) {
                            static int current_button_id = 0;

                            // Only do this once
                            if (current_button_item != 2) {
                                current_button_item = 2;
                                custom_button_name = "saturn/buttons/" + button_array[current_button_id];
                                saturn_button_swap();
                            }

                            string button_name = button_array[current_button_id];
                            if (ImGui::BeginCombo(".png ###button", button_name.c_str()))
                            {
                                for (int o = 0; o < button_array.size(); o++)
                                {
                                    const bool is_button_selected = (current_button_id == o);
                                    button_name = button_array[o];
                                    if (ImGui::Selectable(button_name.c_str(), is_button_selected)) {
                                        current_button_id = o;
                                    }

                                    // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                    if (is_button_selected)
                                        ImGui::SetItemDefaultFocus();
                                }
                                ImGui::EndCombo();
                            }
                            if (ImGui::Button("Set Buttons")) {
                                custom_button_name = "saturn/buttons/" + button_array[current_button_id];
                                saturn_button_swap();
                            }
                            ImGui::Dummy(ImVec2(0, 5));
                        }

                        const char* sideburnStates[] = { "Default", "None", "Custom..." };
                        ImGui::Combo("Sideburns", &selected_sideburn_item, sideburnStates, IM_ARRAYSIZE(sideburnStates));
                        if (selected_sideburn_item == 0 && current_sideburn_item != 0) {
                            current_sideburn_item = 0;
                            custom_sideburn_name = "default";
                            saturn_sideburn_swap();
                        } else if (selected_sideburn_item == 1 && current_sideburn_item != 1) {
                            current_sideburn_item = 1;
                            custom_sideburn_name = "saturn/blank";
                            saturn_sideburn_swap();
                        } 
                        else if (selected_sideburn_item == 2 && sideburn_array.size() > 0) {
                            static int current_sideburn_id = 0;

                            // Only do this once
                            if (current_sideburn_item != 2) {
                                current_sideburn_item = 2;
                                custom_sideburn_name = "saturn/sideburns/" + sideburn_array[current_sideburn_id];
                                saturn_sideburn_swap();
                            }

                            string sideburn_name = sideburn_array[current_sideburn_id];
                            if (ImGui::BeginCombo(".png ###sideburn", sideburn_name.c_str()))
                            {
                                for (int o = 0; o < sideburn_array.size(); o++)
                                {
                                    const bool is_sideburn_selected = (current_sideburn_id == o);
                                    sideburn_name = sideburn_array[o];
                                    if (ImGui::Selectable(sideburn_name.c_str(), is_sideburn_selected)) {
                                        current_sideburn_id = o;
                                    }

                                    // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                    if (is_sideburn_selected)
                                        ImGui::SetItemDefaultFocus();
                                }
                                ImGui::EndCombo();
                            }
                            if (ImGui::Button("Set Sideburns")) {
                                custom_sideburn_name = "saturn/sideburns/" + sideburn_array[current_sideburn_id];
                                saturn_sideburn_swap();
                            }
                            ImGui::Dummy(ImVec2(0, 5));
                        }
                    }

                    ImGui::End();
                    ImGui::PopStyleColor();
                }
                if (configImGui.s_appearance && show_menu_bar) {
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
                    ImGui::Begin("Appearance", NULL, ImGuiWindowFlags_None);

                    ImGui::InputText(".gs", bufname, IM_ARRAYSIZE(bufname));
                    if (ImGui::Button("Save to File")) {
                        apply_cc_from_editor();

                        std::string cc_name = bufname;
                        // We don't want to save a CC named "Mario", as it may cause file issues.
                        if (cc_name != "Mario") {
                            save_cc_file(cc_name);
                        } else {
                            strcpy(bufname, "Sample");
                            save_cc_file("Sample");
                        }

                        load_cc_directory();
                    }
                    if (ImGui::Button("Delete CC")) {
                        delete_cc_file(bufname);
                    } ImGui::SameLine(); HelpMarker(
                        "WARNING: This will delete the CC file as written above. This action is irreversible!");

                    ImGui::Dummy(ImVec2(0, 5));

                    if (ImGui::BeginTabBar("###cc_tabbar", ImGuiTabBarFlags_None)) {
                        if (ImGui::BeginTabItem("CC Editor")) {
                            ImGui::Text("Shirt/Cap");
                            ImGui::ColorEdit4("Hat Main", (float*)&uiHatColor, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_NoLabel);
                            ImGui::ColorEdit4("Hat Shade", (float*)&uiHatShadeColor, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_NoLabel);
                            ImGui::Text("Overalls");
                            ImGui::ColorEdit4("Overalls Main", (float*)&uiOverallsColor, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_NoLabel);
                            ImGui::ColorEdit4("Overalls Shade", (float*)&uiOverallsShadeColor, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_NoLabel);
                            ImGui::Text("Gloves");
                            ImGui::ColorEdit4("Gloves Main", (float*)&uiGlovesColor, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_NoLabel);
                            ImGui::ColorEdit4("Gloves Shade", (float*)&uiGlovesShadeColor, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_NoLabel);
                            ImGui::Text("Shoes");
                            ImGui::ColorEdit4("Shoes Main", (float*)&uiShoesColor, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_NoLabel);
                            ImGui::ColorEdit4("Shoes Shade", (float*)&uiShoesShadeColor, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_NoLabel);
                            ImGui::Text("Skin");
                            ImGui::ColorEdit4("Skin Main", (float*)&uiSkinColor, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_NoLabel);
                            ImGui::ColorEdit4("Skin Shade", (float*)&uiSkinShadeColor, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_NoLabel);
                            ImGui::Text("Hair");
                            ImGui::ColorEdit4("Hair Main", (float*)&uiHairColor, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_NoLabel);
                            ImGui::ColorEdit4("Hair Shade", (float*)&uiHairShadeColor, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_NoLabel);
                            ImGui::EndTabItem();

                            ImGui::Dummy(ImVec2(0, 5));

                            if (ImGui::Button("Apply CC")) {
                                apply_cc_from_editor();
                                strcpy(cc_gameshark, global_color_to_cc().c_str());
                            }
                        }
                        if (ImGui::BeginTabItem("GameShark")) {
                            ImGui::InputTextMultiline("###gameshark_box", cc_gameshark, IM_ARRAYSIZE(cc_gameshark), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 25), ImGuiInputTextFlags_CharsUppercase);

                            ImGui::Dummy(ImVec2(0, 5));

                            if (ImGui::Button("Paste GS Code")) {
                                std::string input = cc_gameshark;
                                load_cc_data(input);

                                uiHatColor = ImVec4(float(defaultColorHatRLight) / 255.0f, float(defaultColorHatGLight) / 255.0f, float(defaultColorHatBLight) / 255.0f, 255.0f / 255.0f);
                                uiHatShadeColor = ImVec4(float(defaultColorHatRDark) / 255.0f, float(defaultColorHatGDark) / 255.0f, float(defaultColorHatBDark) / 255.0f, 255.0f / 255.0f);
                                uiOverallsColor = ImVec4(float(defaultColorOverallsRLight) / 255.0f, float(defaultColorOverallsGLight) / 255.0f, float(defaultColorOverallsBLight) / 255.0f, 255.0f / 255.0f);
                                uiOverallsShadeColor = ImVec4(float(defaultColorOverallsRDark) / 255.0f, float(defaultColorOverallsGDark) / 255.0f, float(defaultColorOverallsBDark) / 255.0f, 255.0f / 255.0f);
                                uiGlovesColor = ImVec4(float(defaultColorGlovesRLight) / 255.0f, float(defaultColorGlovesGLight) / 255.0f, float(defaultColorGlovesBLight) / 255.0f, 255.0f / 255.0f);
                                uiGlovesShadeColor = ImVec4(float(defaultColorGlovesRDark) / 255.0f, float(defaultColorGlovesGDark) / 255.0f, float(defaultColorGlovesBDark) / 255.0f, 255.0f / 255.0f);
                                uiShoesColor = ImVec4(float(defaultColorShoesRLight) / 255.0f, float(defaultColorShoesGLight) / 255.0f, float(defaultColorShoesBLight) / 255.0f, 255.0f / 255.0f);
                                uiShoesShadeColor = ImVec4(float(defaultColorShoesRDark) / 255.0f, float(defaultColorShoesGDark) / 255.0f, float(defaultColorShoesBDark) / 255.0f, 255.0f / 255.0f);
                                uiSkinColor = ImVec4(float(defaultColorSkinRLight) / 255.0f, float(defaultColorSkinGLight) / 255.0f, float(defaultColorSkinBLight) / 255.0f, 255.0f / 255.0f);
                                uiSkinShadeColor = ImVec4(float(defaultColorSkinRDark) / 255.0f, float(defaultColorSkinGDark) / 255.0f, float(defaultColorSkinBDark) / 255.0f, 255.0f / 255.0f);
                                uiHairColor = ImVec4(float(defaultColorHairRLight) / 255.0f, float(defaultColorHairGLight) / 255.0f, float(defaultColorHairBLight) / 255.0f, 255.0f / 255.0f);
                                uiHairShadeColor = ImVec4(float(defaultColorHairRDark) / 255.0f, float(defaultColorHairGDark) / 255.0f, float(defaultColorHairBDark) / 255.0f, 255.0f / 255.0f);

                                strcpy(cc_gameshark, global_color_to_cc().c_str());
                                strcpy(bufname, "Sample");
                            }

                            ImGui::EndTabItem();
                        }
                        ImGui::EndTabBar();
                    }

                    ImGui::End();
                    ImGui::PopStyleColor();
                }

                //ImGui::ShowDemoWindow();

                if (configImGui.s_options && show_menu_bar) {
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
                    ImGui::Begin("Settings", NULL, ImGuiWindowFlags_None);

                    if (ImGui::CollapsingHeader("Graphics")) {
                        /*
                        if (ImGui::Button("Toggle Fullscreen")) {
                            configWindow.fullscreen = !configWindow.fullscreen;
                            configWindow.settings_changed = true;
                        }
                        */
                        if (!configWindow.fullscreen) {
                            if (ImGui::Button("Reset Window Size")) {
                                configWindow.w = 1280;
                                configWindow.h = 768;   // 720 + 48 for the top bar
                                SDL_SetWindowSize(window, 1280, 768);
                            }
                        }
                        ImGui::Checkbox("VSync", &configWindow.vsync);
                        ImGui::Text("Internal Multiplier");
                        ImGui::SliderFloat("###internal_multiplier", &configWindow.multiplier, 1.0f, 4.0f);
                        ImGui::Checkbox("Jabo Mode", &configImGui.jaboMode);
                        ImGui::SameLine(); HelpMarker(
                            "Mimics Project64's widescreen stretching");
                        ImGui::Text("Graphics Quality");
                        const char* lod_modes[] = { "Auto", "Low", "High" };
                        ImGui::Combo("###lod_modes", (int*)&configLODMode, lod_modes, IM_ARRAYSIZE(lod_modes));
                        ImGui::Text("Texture Filtering");
                        const char* texture_filters[] = { "Nearest", "Linear", "Three-point" };
                        ImGui::Combo("###texture_filters", (int*)&configFiltering, texture_filters, IM_ARRAYSIZE(texture_filters));
                    }
                    if (ImGui::CollapsingHeader("Audio")) {
                        ImGui::Text("Volume");
                        ImGui::SliderInt("Master", (int*)&configMasterVolume, 0, MAX_VOLUME);
                        ImGui::SliderInt("SFX", (int*)&configSfxVolume, 0, MAX_VOLUME);
                        ImGui::SliderInt("Music", (int*)&configMusicVolume, 0, MAX_VOLUME);
                        ImGui::SliderInt("Environment", (int*)&configEnvVolume, 0, MAX_VOLUME);
                        ImGui::Checkbox("Enable Voices", &configVoicesEnabled);
                    }
                    if (ImGui::CollapsingHeader("Gameplay")) {

                        ImGui::Text("Game Language");
                        char *configlangName = new char[255];
	                    sprintf(configlangName,"%ls",Moon::languages[configLanguage]->name.c_str());
                        if (ImGui::BeginCombo("###game_language", configlangName))
                        {
                            for (int i = 0; i < Moon::languages.size(); i++)
                            {
                                bool langBoxSelected = (configLanguage == i);
                                char *langName = new char[255];
	                            sprintf(langName,"%ls",Moon::languages[i]->name.c_str());

                                if (ImGui::Selectable(langName, langBoxSelected)) {
                                    Moon::setCurrentLanguage(Moon::languages[i]);
                                    configLanguage = i;
                                    configlangName = langName;
                                }

                                if (langBoxSelected)
                                    ImGui::SetItemDefaultFocus();
                            }
                            ImGui::EndCombo();
                        }
                        ImGui::Dummy(ImVec2(0, 5));

#ifdef DISCORDRPC
                        ImGui::Checkbox("Discord Activity Status", &configDiscordRPC);
                        ImGui::Dummy(ImVec2(0, 5));
#endif

                        ImGui::Text("Rumble Strength");
                        ImGui::SliderInt("###rumble_strength", (int*)&configRumbleStrength, 0, 50);
                        ImGui::Checkbox("Precache Textures", &configPrecacheRes);
                        ImGui::Checkbox("Skip Intro", &configSkipIntro);
                        ImGui::Checkbox("Enable Yoshi", &enable_yoshi);
                    }

                    ImGui::End();
                    ImGui::PopStyleColor();
                }

                if(configImGui.texture_debug && show_menu_bar) {
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
                    ImGui::Begin("Loaded textures", NULL, ImGuiWindowFlags_None);

                    float value = 0.0f;

                    if (ImGui::BeginTable("texture_pool", 2, ImGuiTableFlags_Borders)) {
                        ImGui::TableSetupColumn("Image", ImGuiTableColumnFlags_WidthFixed, 64.0f);
                        ImGui::TableSetupColumn("Path");
                        // ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 64.0f);
                        ImGui::TableHeadersRow();
                        if (ImGui::BeginPopupContextItem("test")) {
                            if (ImGui::Selectable("Set to zero")) value = 0.0f;
                            if (ImGui::Selectable("Set to PI")) value = 3.1415f;
                            ImGui::SetNextItemWidth(-FLT_MIN);
                            ImGui::DragFloat("##Value", &value, 0.1f, 0.0f, 0.0f);
                            ImGui::EndPopup();
                        }
                        for(auto &entry : textureMap){
                            if(entry.second == nullptr) continue;
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            float fw = entry.second->width <= 0 ? 32.0f : entry.second->width;
                            float fh = entry.second->height <= 0 ? 32.0f : entry.second->height;
                            ImGui::Image((ImTextureID) entry.second->texture_id, ImVec2(64, 64 * fh / fw ));
                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text("%s", entry.second->texture_addr);
                            // ImGui::TableSetColumnIndex(2);

                            // if(ImGui::Button(ICON_MD_EDIT, ImVec2(64, 64))){
                            //     ImGui::OpenPopup("test");
                            //     cout << entry.second->texture_addr << endl;
                            //     ImGui::OpenPopupOnItemClick("popup", ImGuiPopupFlags_MouseButtonRight);
                            // }
                        }
                        ImGui::EndTable();
                    }
                    ImGui::End();
                    ImGui::PopStyleColor();
                }

                ImGui::Render();
                GLint last_program;
                glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
                glUseProgram(0);
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                glUseProgram(last_program);
            }});
        }
    }
}