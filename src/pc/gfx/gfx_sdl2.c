#ifdef __MINGW32__
#define FOR_WINDOWS 1
#else
#define FOR_WINDOWS 0
#endif

#if FOR_WINDOWS
#define GLEW_STATIC
#include <GL/glew.h>
#include <SDL2/SDL.h>
#define GL_GLEXT_PROTOTYPES 1
#include <SDL2/SDL_opengl.h>
#else
#include <SDL2/SDL.h>
#define GL_GLEXT_PROTOTYPES 1

#ifdef OSX_BUILD
#include <SDL2/SDL_opengl.h>
#else
#include <SDL2/SDL_opengles2.h>
#endif

#endif // End of OS-Specific GL defines

#include <stdio.h>

#include "gfx_window_manager_api.h"
#include "gfx_screen_config.h"
#include "../pc_main.h"
#include "../configfile.h"
#include "../cliopts.h"

#include "src/pc/controller/controller_keyboard.h"

// TODO: figure out if this shit even works
#ifdef VERSION_EU
# define FRAMERATE 25
#else
# define FRAMERATE 30
#endif

static struct {
    SDL_GLContext ctx;
    SDL_Window*   wnd;
    Uint64        frametime;
    int           refresh_rate;
    int           swap_interval;
    bool          exiting_fullscreen;
    int           inverted_scancode_table[512];
} gContext = { 0 };

const SDL_Scancode windows_scancode_table[] =
{
	/*	0						1							2							3							4						5							6							7 */
	/*	8						9							A							B							C						D							E							F */
	SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_ESCAPE,		SDL_SCANCODE_1,				SDL_SCANCODE_2,				SDL_SCANCODE_3,			SDL_SCANCODE_4,				SDL_SCANCODE_5,				SDL_SCANCODE_6,			/* 0 */
	SDL_SCANCODE_7,				SDL_SCANCODE_8,				SDL_SCANCODE_9,				SDL_SCANCODE_0,				SDL_SCANCODE_MINUS,		SDL_SCANCODE_EQUALS,		SDL_SCANCODE_BACKSPACE,		SDL_SCANCODE_TAB,		/* 0 */

	SDL_SCANCODE_Q,				SDL_SCANCODE_W,				SDL_SCANCODE_E,				SDL_SCANCODE_R,				SDL_SCANCODE_T,			SDL_SCANCODE_Y,				SDL_SCANCODE_U,				SDL_SCANCODE_I,			/* 1 */
	SDL_SCANCODE_O,				SDL_SCANCODE_P,				SDL_SCANCODE_LEFTBRACKET,	SDL_SCANCODE_RIGHTBRACKET,	SDL_SCANCODE_RETURN,	SDL_SCANCODE_LCTRL,			SDL_SCANCODE_A,				SDL_SCANCODE_S,			/* 1 */

	SDL_SCANCODE_D,				SDL_SCANCODE_F,				SDL_SCANCODE_G,				SDL_SCANCODE_H,				SDL_SCANCODE_J,			SDL_SCANCODE_K,				SDL_SCANCODE_L,				SDL_SCANCODE_SEMICOLON,	/* 2 */
	SDL_SCANCODE_APOSTROPHE,	SDL_SCANCODE_GRAVE,			SDL_SCANCODE_LSHIFT,		SDL_SCANCODE_BACKSLASH,		SDL_SCANCODE_Z,			SDL_SCANCODE_X,				SDL_SCANCODE_C,				SDL_SCANCODE_V,			/* 2 */

	SDL_SCANCODE_B,				SDL_SCANCODE_N,				SDL_SCANCODE_M,				SDL_SCANCODE_COMMA,			SDL_SCANCODE_PERIOD,	SDL_SCANCODE_SLASH,			SDL_SCANCODE_RSHIFT,		SDL_SCANCODE_PRINTSCREEN,/* 3 */
	SDL_SCANCODE_LALT,			SDL_SCANCODE_SPACE,			SDL_SCANCODE_CAPSLOCK,		SDL_SCANCODE_F1,			SDL_SCANCODE_F2,		SDL_SCANCODE_F3,			SDL_SCANCODE_F4,			SDL_SCANCODE_F5,		/* 3 */

	SDL_SCANCODE_F6,			SDL_SCANCODE_F7,			SDL_SCANCODE_F8,			SDL_SCANCODE_F9,			SDL_SCANCODE_F10,		SDL_SCANCODE_NUMLOCKCLEAR,	SDL_SCANCODE_SCROLLLOCK,	SDL_SCANCODE_HOME,		/* 4 */
	SDL_SCANCODE_UP,			SDL_SCANCODE_PAGEUP,		SDL_SCANCODE_KP_MINUS,		SDL_SCANCODE_LEFT,			SDL_SCANCODE_KP_5,		SDL_SCANCODE_RIGHT,			SDL_SCANCODE_KP_PLUS,		SDL_SCANCODE_END,		/* 4 */

	SDL_SCANCODE_DOWN,			SDL_SCANCODE_PAGEDOWN,		SDL_SCANCODE_INSERT,		SDL_SCANCODE_DELETE,		SDL_SCANCODE_UNKNOWN,	SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_NONUSBACKSLASH,SDL_SCANCODE_F11,		/* 5 */
	SDL_SCANCODE_F12,			SDL_SCANCODE_PAUSE,			SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_LGUI,			SDL_SCANCODE_RGUI,		SDL_SCANCODE_APPLICATION,	SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_UNKNOWN,	/* 5 */

	SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_F13,		SDL_SCANCODE_F14,			SDL_SCANCODE_F15,			SDL_SCANCODE_F16,		/* 6 */
	SDL_SCANCODE_F17,			SDL_SCANCODE_F18,			SDL_SCANCODE_F19,			SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_UNKNOWN,	SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_UNKNOWN,	/* 6 */

	SDL_SCANCODE_INTERNATIONAL2,		SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_INTERNATIONAL1,		SDL_SCANCODE_UNKNOWN,	SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_UNKNOWN,	/* 7 */
	SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_INTERNATIONAL4,		SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_INTERNATIONAL5,		SDL_SCANCODE_UNKNOWN,	SDL_SCANCODE_INTERNATIONAL3,		SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_UNKNOWN	/* 7 */
};

const SDL_Scancode scancode_rmapping_extended[][2] = {
    {SDL_SCANCODE_KP_ENTER, SDL_SCANCODE_RETURN},
    {SDL_SCANCODE_RALT, SDL_SCANCODE_LALT},
    {SDL_SCANCODE_RCTRL, SDL_SCANCODE_LCTRL},
    {SDL_SCANCODE_KP_DIVIDE, SDL_SCANCODE_SLASH},
    //{SDL_SCANCODE_KP_PLUS, SDL_SCANCODE_CAPSLOCK}
};

const SDL_Scancode scancode_rmapping_nonextended[][2] = {
    {SDL_SCANCODE_KP_7, SDL_SCANCODE_HOME},
    {SDL_SCANCODE_KP_8, SDL_SCANCODE_UP},
    {SDL_SCANCODE_KP_9, SDL_SCANCODE_PAGEUP},
    {SDL_SCANCODE_KP_4, SDL_SCANCODE_LEFT},
    {SDL_SCANCODE_KP_6, SDL_SCANCODE_RIGHT},
    {SDL_SCANCODE_KP_1, SDL_SCANCODE_END},
    {SDL_SCANCODE_KP_2, SDL_SCANCODE_DOWN},
    {SDL_SCANCODE_KP_3, SDL_SCANCODE_PAGEDOWN},
    {SDL_SCANCODE_KP_0, SDL_SCANCODE_INSERT},
    {SDL_SCANCODE_KP_PERIOD, SDL_SCANCODE_DELETE},
    {SDL_SCANCODE_KP_MULTIPLY, SDL_SCANCODE_PRINTSCREEN}
};

#define IS_FULLSCREEN \
    (SDL_GetWindowFlags(gContext.wnd) & SDL_WINDOW_FULLSCREEN_DESKTOP)

static void gfx_sdl_update_vsync() {
    static int current_swap_interval = -1;

    if (!configWindow.vsync) {
        gContext.swap_interval = 0;
    } else if (
        gContext.refresh_rate < FRAMERATE || gContext.refresh_rate % FRAMERATE
    ) {
        // Soft disable vsync in environemts where display's refresh rate
        // information is unavailable or it is not a multiple of the game's
        // fixed framerate
        if (gContext.swap_interval != 0) {
            SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Vsync is not available");
            gContext.swap_interval = 0;
        }
    } else {
        gContext.swap_interval = gContext.refresh_rate / FRAMERATE;
    }

    // Avoid unecessary changes
    if (gContext.swap_interval == current_swap_interval) return;
    if (
        current_swap_interval != -1
        && current_swap_interval != 0 && gContext.swap_interval != 0
    ) {
        // Allow an unsynchronized frame to render to avoid possible jitter
        // introduced by immediatly changing swap_interval
        current_swap_interval = 0;
        SDL_GL_SetSwapInterval(0);
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Jitter fix");
        return;
    }

    if (
        SDL_GL_SetSwapInterval(gContext.swap_interval) && gContext.swap_interval
    ) {
        // Failed to set intended swap interval, force disabled vsync
        configWindow.vsync = false;
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Failed to enable vsync");
        return gfx_sdl_update_vsync(); // Recurse once to reset vsync
    }

    // Cache current swap valuez
    current_swap_interval = gContext.swap_interval;

    if (gContext.swap_interval)
        SDL_LogDebug(
            SDL_LOG_CATEGORY_VIDEO, "Vsync enable: %d", gContext.swap_interval
        );
    else
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Vsync disabled");
}

static void gfx_sdl_set_fullscreen() {
    if (configWindow.fullscreen == IS_FULLSCREEN)
        return;
    if (configWindow.fullscreen) {
        SDL_SetWindowFullscreen(gContext.wnd, SDL_WINDOW_FULLSCREEN_DESKTOP);
        SDL_ShowCursor(SDL_DISABLE);
    } else {
        SDL_SetWindowFullscreen(gContext.wnd, 0);
        SDL_ShowCursor(SDL_ENABLE);
        gContext.exiting_fullscreen = true;
    }
}

static void gfx_sdl_update_refresh_rate() {
    //! FIXME:
    // SDL caches the display's current_mode at initialization and only updates
    // it on explicit SDL mode change calls. So, if any external change happens
    // it will report wrong values.
    // Research a way to force SDL to update the display's current_mode on
    // external changes.
    // https://stackoverflow.com/questions/39810237/get-the-updated-screen-resolution-in-sdl2
    SDL_DisplayMode current_display_mode = { 0 };
    int current_display = SDL_GetWindowDisplayIndex(gContext.wnd);
    if (current_display < 0
        || SDL_GetCurrentDisplayMode(current_display, &current_display_mode)
    ) {
        // Failed to query display refresh rate, defaults to 0 to disable vsync
        gContext.refresh_rate = 0;
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Failed to query refresh rate");
        return;
    }

    gContext.refresh_rate = current_display_mode.refresh_rate;
}

static void gfx_sdl_reset_dimension_and_pos() {
    if (gContext.exiting_fullscreen) {
        gContext.exiting_fullscreen = false;
    } else if (configWindow.reset) {
        configWindow.x = SDL_WINDOWPOS_CENTERED;
        configWindow.y = SDL_WINDOWPOS_CENTERED;
        configWindow.w = DESIRED_SCREEN_WIDTH;
        configWindow.h = DESIRED_SCREEN_HEIGHT;
        configWindow.vsync = true;
        configWindow.reset = false;

        if (IS_FULLSCREEN) {
            configWindow.fullscreen = false;
            return;
        }
    } else {
        return;
    }

    SDL_SetWindowSize(gContext.wnd, configWindow.w, configWindow.h);
    SDL_SetWindowPosition(gContext.wnd, configWindow.x, configWindow.y);
}

static void gfx_sdl_init(void) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_LogSetAllPriority(getenv("DEBUG") ? SDL_LOG_PRIORITY_DEBUG : SDL_LOG_PRIORITY_INFO);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    #ifdef USE_GLES
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);  // These attributes allow for hardware acceleration on RPis.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    #endif

    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    if (gCLIOpts.FullScreen == 1)
        configWindow.fullscreen = true;
    else if (gCLIOpts.FullScreen == 2)
        configWindow.fullscreen = false;

    const char* window_title = 
    #ifndef USE_GLES
    "Super Mario 64 PC port (OpenGL)";
    #else
    "Super Mario 64 PC port (OpenGL_ES2)";
    #endif

    gContext.wnd = SDL_CreateWindow(
        window_title,
        configWindow.x, configWindow.y, configWindow.w, configWindow.h,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    gContext.ctx = SDL_GL_CreateContext(gContext.wnd);

    gfx_sdl_update_refresh_rate();
    gfx_sdl_update_vsync();
    gfx_sdl_set_fullscreen();

    // Time that a game's frame should take relative to SDL performance frequency
    gContext.frametime = SDL_GetPerformanceFrequency() / FRAMERATE;

    for (size_t i = 0; i < sizeof(windows_scancode_table) / sizeof(SDL_Scancode); i++) {
        gContext.inverted_scancode_table[windows_scancode_table[i]] = i;
    }

    for (size_t i = 0; i < sizeof(scancode_rmapping_extended) / sizeof(scancode_rmapping_extended[0]); i++) {
        gContext.inverted_scancode_table[scancode_rmapping_extended[i][0]] = gContext.inverted_scancode_table[scancode_rmapping_extended[i][1]] + 0x100;
    }

    for (size_t i = 0; i < sizeof(scancode_rmapping_nonextended) / sizeof(scancode_rmapping_nonextended[0]); i++) {
        gContext.inverted_scancode_table[scancode_rmapping_nonextended[i][0]] = gContext.inverted_scancode_table[scancode_rmapping_nonextended[i][1]];
        gContext.inverted_scancode_table[scancode_rmapping_nonextended[i][1]] += 0x100;
    }
}

static void gfx_sdl_main_loop(void (*run_one_game_iter)(void)) {
    Uint64 start, elapsed;
    while (1) {
        start = SDL_GetPerformanceCounter();
        run_one_game_iter();

        if (gContext.swap_interval != 0) continue;

        elapsed = SDL_GetPerformanceCounter() - start;
        while (elapsed < gContext.frametime) {
            SDL_Delay(0);
            elapsed = SDL_GetPerformanceCounter() - start;
        }
    }
}

static void gfx_sdl_get_dimensions(uint32_t *width, uint32_t *height) {
    SDL_GetWindowSize(gContext.wnd, width, height);
}

static int translate_scancode(int scancode) {
    if (scancode < 512) {
        return gContext.inverted_scancode_table[scancode];
    } else {
        return 0;
    }
}

static void gfx_sdl_onkeydown(int scancode) {
    keyboard_on_key_down(translate_scancode(scancode));

    const Uint8 *state = SDL_GetKeyboardState(NULL);

    if (state[SDL_SCANCODE_LALT] && state[SDL_SCANCODE_RETURN])
        configWindow.fullscreen = !configWindow.fullscreen;
    else if (state[SDL_SCANCODE_ESCAPE] && configWindow.fullscreen)
        configWindow.fullscreen = false;
}

static void gfx_sdl_onkeyup(int scancode) {
    keyboard_on_key_up(translate_scancode(scancode));
}

static void gfx_sdl_handle_events(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
#ifndef TARGET_WEB
            // Scancodes are broken in Emscripten SDL2: https://bugzilla.libsdl.org/show_bug.cgi?id=3259
            case SDL_KEYDOWN:
                gfx_sdl_onkeydown(event.key.keysym.scancode);
                break;
            case SDL_KEYUP:
                gfx_sdl_onkeyup(event.key.keysym.scancode);
                break;
#endif
            case SDL_WINDOWEVENT:
                gfx_sdl_update_refresh_rate();
                if (!(IS_FULLSCREEN || gContext.exiting_fullscreen)) {
                    // TODO: Check if this makes sense to be included in the Web build
                    switch (event.window.event) {
                        case SDL_WINDOWEVENT_MOVED:
                            configWindow.x = event.window.data1;
                            configWindow.y = event.window.data2;
                            break;
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                            configWindow.w = event.window.data1;
                            configWindow.h = event.window.data2;
                            break;
                    }
                }
                break;
            case SDL_QUIT:
                game_exit();
                break;
        }
    }

    gfx_sdl_update_vsync();
    gfx_sdl_reset_dimension_and_pos();
    gfx_sdl_set_fullscreen();
}

static bool gfx_sdl_start_frame(void) {
    return true;
}

static void gfx_sdl_swap_buffers_begin(void) {
    SDL_GL_SwapWindow(gContext.wnd);
}

static void gfx_sdl_swap_buffers_end(void) {
}

static double gfx_sdl_get_time(void) {
    return 0.0;
}


static void gfx_sdl_shutdown(void) {
    if (SDL_WasInit(0)) {
        if (gContext.ctx) { SDL_GL_DeleteContext(gContext.ctx); gContext.ctx = NULL; }
        if (gContext.wnd) { SDL_DestroyWindow(gContext.wnd); gContext.wnd = NULL; }
        SDL_Quit();
    }
}

struct GfxWindowManagerAPI gfx_sdl = {
    gfx_sdl_init,
    gfx_sdl_main_loop,
    gfx_sdl_get_dimensions,
    gfx_sdl_handle_events,
    gfx_sdl_start_frame,
    gfx_sdl_swap_buffers_begin,
    gfx_sdl_swap_buffers_end,
    gfx_sdl_get_time,
    gfx_sdl_shutdown
};
