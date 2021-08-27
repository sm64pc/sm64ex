#ifdef UNUSED_FEAT
#include "skybox.h"
#include "moon/mod-engine/hooks/hook.h"

#ifdef __MINGW32__
# define FOR_WINDOWS 1
#else
# define FOR_WINDOWS 0
#endif

#if FOR_WINDOWS || defined(OSX_BUILD)
# define GLEW_STATIC
# include <GL/glew.h>
#endif

#include <SDL2/SDL.h>

#ifdef TARGET_SWITCH
    #include "glad/glad.h"
#else
    #define GL_GLEXT_PROTOTYPES 1
    #ifdef USE_GLES
        # include <SDL2/SDL_opengles2.h>
    #else
        # include <SDL2/SDL_opengl.h>
    #endif
#endif

#include "moon/utils/moon-env.h"
#include "./shader.h"
#include "./camera.h"
#include "pc/configfile.h"

#include <string>
using namespace std;

float quadVertices[] = {
        // positions          // texture Coords
         5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
        -5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

         5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
         5.0f, -0.5f, -5.0f,  2.0f, 2.0f
    };

unsigned int quadVAO, quadVBO;
Shader screen("src/moon/renderer/shaders/screen.vs", "src/moon/renderer/shaders/screen.fs");
Shader screenShader("src/moon/renderer/shaders/framebuffer.vs", "src/moon/renderer/shaders/framebuffer.fs");
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

namespace MoonRenderer {
    void setupSkyboxRenderer(string status){
        if(status == "PreStartup"){
            Moon::registerHookListener({.hookName = GFX_INIT, .callback = [](HookCall call){
                glGenVertexArrays(1, &quadVAO);
                glGenBuffers(1, &quadVBO);
                glBindVertexArray(quadVAO);
                glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
                screen.load();
                screenShader.load();

                screen.use();
                screen.setInt("texture1", 0);

                screenShader.use();
                screenShader.setInt("screenTexture", stoi(MoonInternal::getEnvironmentVar("framebuffer")));
            }});
            Moon::registerHookListener({.hookName = GFX_PRE_START_FRAME, .callback = [](HookCall call){
                screen.use();
                glm::mat4 model = glm::mat4(1.0f);
                glm::mat4 view = camera.GetViewMatrix();
                glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)configWindow.internal_w / (float)configWindow.internal_h, 0.1f, 100.0f);
                screen.setMat4("view", view);
                screen.setMat4("projection", projection);

                screenShader.use();
                glBindVertexArray(quadVAO);
                glBindTexture(GL_TEXTURE_2D, stoi(MoonInternal::getEnvironmentVar("framebuffer")));
                glDrawArrays(GL_TRIANGLES, 0, 6);
                glUseProgram(0);
            }});
        }
    }
}
#endif