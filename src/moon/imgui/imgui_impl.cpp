#include "imgui_impl.h"

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
}

using namespace std;

bool showMenu = true;
bool showWindowMoon = false;
bool showWindowDemo = false;

SDL_Window* window = nullptr;
ImGuiIO io;

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

    void setupImGuiModule(string status) {
        MoonInternal::setupWindowHook(status);
        if(status == "PreStartup"){
            Moon::registerHookListener({.hookName = WINDOW_API_INIT, .callback = [](HookCall call){
                const char* glsl_version = "#version 120";
                ImGuiContext* ctx = ImGui::CreateContext();
                ImGui::SetCurrentContext(ctx);
                io = ImGui::GetIO(); (void)io;
                io.WantSetMousePos = false;
                io.ConfigWindowsMoveFromTitleBarOnly = true;
                for (auto entry = Moon::fonts.begin(); entry != Moon::fonts.end(); entry++){
                    ImFontConfig font_cfg;
                    font_cfg.FontDataOwnedByAtlas = false;
                    io.Fonts->AddFontFromMemoryTTF((void*) entry->second->data, entry->second->size, 18.f, &font_cfg);
                }
                ImGui::StyleColorsLightGreen();

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
                switch (ev->type){
                    case SDL_KEYDOWN:
                        if(ev->key.keysym.sym == SDLK_F12)
                            showMenu = !showMenu;
                        break;
                }
            }});


            Moon::registerHookListener({ GFX_POST_END_FRAME, [](HookCall call){
                // recv(socketID, NULL, 1, MSG_PEEK | MSG_DONTWAIT) != 0
                bool retval = 0;

                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplSDL2_NewFrame(window);
                ImGui::NewFrame();
            #ifdef TARGET_SWITCH
                MoonNX::handleVirtualKeyboard("FrameUpdate");
            #endif
                MoonInternal::bindHook(IMGUI_API_DRAW);
                MoonInternal::initBindHook(0);
                MoonInternal::callBindHook(0);
            #ifdef GAME_DEBUG
                if(showWindowDemo)
                    ImGui::ShowDemoWindow(NULL);

                if(showMenu){
                    ImGui::BeginMainMenuBar();
                    ImGui::MenuItem("Moon64", NULL, &showWindowMoon);
                    ImGui::MenuItem("ImGui Demo", NULL, &showWindowDemo);
                    ImGui::EndMainMenuBar();

                    if (showWindowMoon){
                        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
                        ImGui::Begin("Moon64 Game Stats", NULL, ImGuiWindowFlags_None);
                        // ImGui::SetWindowPos(ImVec2(10, 35));
                        ImGui::Text("Platform: " PLATFORM " (" RAPI_NAME ")");
                        ImGui::Text("Status: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                        ImGui::Text("Version: " GIT_BRANCH " " GIT_HASH);
                        ImGui::Text("NXLink: %d\n", retval);
                        ImGui::End();
                        ImGui::PopStyleColor();
                    }
                }
            #endif
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