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
}

#include "pc/configfile.h"

#define DEFAULT_FONT "monogram.ttf"

using namespace std;

SDL_Window* window = nullptr;
ImGuiIO* io = nullptr;

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

namespace MoonInternal {

    map<string, ImFont*> fontMap;

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

            #ifdef TARGET_SWITCH
                MoonNX::handleVirtualKeyboard("Init");
            #endif
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
                ImGui::SetNextWindowSize(viewport->WorkSize);
                ImGui::SetNextWindowViewport(viewport->ID);
                window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;
                window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
                ImGui::Begin("Main - Deck", nullptr, window_flags);
                ImGui::PopStyleVar();
                ImGuiID dockspace_id = ImGui::GetID("main_dock");

                if (!ImGui::DockBuilderGetNode(dockspace_id)) {
                    ImGui::DockBuilderRemoveNode(dockspace_id);
                    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_None);

                    ImGui::DockBuilderDockWindow("Game", dockspace_id);

                    ImGui::DockBuilderFinish(dockspace_id);
                }

                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

                if (ImGui::BeginMenuBar()) {
                    TextureData* tmp = forceTextureLoad("mod-icons://Moon64");
                    if(tmp != nullptr) {
                        ImGui::SetCursorPos(ImVec2(0, 0));
                        ImGui::Image((ImTextureID) tmp->texture_id, ImVec2(25.0f, 25.0f));
                        ImGui::SameLine();
                    }
                    ImGui::Text("Moon64");
                    ImGui::SameLine();
                    ImGui::Separator();
                    if (ImGui::BeginMenu("View")) {
                        ImGui::MenuItem("Moon64", NULL, &configImGui.moon64);
                        ImGui::MenuItem("Textures", NULL, &configImGui.texture_debug);
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }
                ImGui::End();

                ImGui::SetNextWindowSize(ImVec2(configWindow.w, configWindow.h), ImGuiCond_FirstUseEver);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
                ImGui::Begin("Game", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
                ImVec2 size = ImGui::GetContentRegionAvail();
                configWindow.internal_w = size.x;
                configWindow.internal_h = size.y;

                int fbuf = stoi(MoonInternal::getEnvironmentVar("framebuffer"));
                // ImGui::Image((ImTextureID) fbuf, size);
                ImGui::ImageRotated((ImTextureID) fbuf, ImVec2(0, 0), size, 180.0f);
                ImGui::End();
                ImGui::PopStyleVar();

                if (configImGui.moon64){
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
                    ImGui::Begin("Moon64 Game Stats", NULL, ImGuiWindowFlags_None);

                    ImGui::Text("Platform: " PLATFORM " (" RAPI_NAME ")");
                    ImGui::Text("Status: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                    ImGui::Text("Version: " GIT_BRANCH " " GIT_HASH);
                    ImGui::Text("Addons: %d\n", Moon::addons.size());
                    ImGui::Text("Resolution: %.0fx%.0f\n", configWindow.w * configWindow.multiplier, configWindow.h * configWindow.multiplier);
                    ImGui::Text("Internal Resolution:");
                    ImGui::SliderFloat("Mul", &configWindow.multiplier, 0.0f, 10.0f);
                    ImGui::End();
                    ImGui::PopStyleColor();
                }

                if(configImGui.texture_debug) {
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