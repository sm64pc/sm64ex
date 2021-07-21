#include "imgui_impl.h"

#include <string>
#include <iostream>

#include "moon/libs/imgui/imgui.h"
#include "moon/libs/imgui/imgui_impl_sdl.h"
#include "moon/libs/imgui/imgui_impl_opengl3.h"
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

#define GL_GLEXT_PROTOTYPES 1
#ifdef USE_GLES
# include <SDL2/SDL_opengles2.h>
#else
# include <SDL2/SDL_opengl.h>
#endif

extern "C" {
#include "pc/gfx/gfx_pc.h"
}

using namespace std;

bool showWindow = true;

SDL_Window* window = nullptr;
ImGuiIO io;

namespace MoonInternal {

    void setupImGuiModule(string status) {
        if(status == "PreStartup"){
            Moon::registerHookListener({.hookName = WINDOW_API_INIT, .callback = [](HookCall call){
                const char* glsl_version = "#version 120";
                ImGuiContext* ctx = ImGui::CreateContext();
                ImGui::SetCurrentContext(ctx);
                io = ImGui::GetIO(); (void)io;
                for (auto entry = Moon::fonts.begin(); entry != Moon::fonts.end(); entry++){
                    ImFontConfig font_cfg;
                    font_cfg.FontDataOwnedByAtlas = false;
                    io.Fonts->AddFontFromMemoryTTF((void*) entry->second->data, entry->second->size, 18.f, &font_cfg);
                    cout << "Loading font: " << entry->first << endl;
                }
                ImGui::StyleColorsLightGreen();
                io.ConfigWindowsMoveFromTitleBarOnly = true;

                window = (SDL_Window*) call.baseArgs["window"];
                ImGui_ImplSDL2_InitForOpenGL(window, call.baseArgs["context"]);
                ImGui_ImplOpenGL3_Init(glsl_version);
            }});

            Moon::registerHookListener({.hookName = WINDOW_API_HANDLE_EVENTS, .callback = [](HookCall call){
                SDL_Event* ev = (SDL_Event*) call.baseArgs["event"];
                ImGui_ImplSDL2_ProcessEvent(ev);
                switch (ev->type){
                    case SDL_KEYDOWN:
                        if(ev->key.keysym.sym == SDLK_F12)
                            showWindow = !showWindow;
                        break;
                }
            }});

            Moon::registerHookListener({ GFX_POST_END_FRAME, [](HookCall call){
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplSDL2_NewFrame(window);
                ImGui::NewFrame();

                static float f = 0.0f;
                static int counter = 0;

                if(showWindow){
                    ImGui::Begin("Moon64 Game Stats");
                    ImGui::Text("Framerate: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                    ImGui::Text("Branch: " GIT_BRANCH);
                    ImGui::Text("Commit: " GIT_HASH);
                    ImGui::End();
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