#ifdef WAPI_SDL2

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

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

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

static SDL_Window *wnd;
static SDL_GLContext ctx = NULL;
static int inverted_scancode_table[512];

static kb_callback_t kb_key_down = NULL;
static kb_callback_t kb_key_up = NULL;
static void (*kb_all_keys_up)(void) = NULL;

// whether to use timer for frame control
static bool use_timer = true;
// time between consequtive game frames, in perf counter ticks
static double frame_time = 0.0; // set in init()
// GetPerformanceFrequency
static double perf_freq = 0.0;

const SDL_Scancode windows_scancode_table[] = {
  /*  0                        1                            2                         3                            4                     5                            6                            7  */
  /*  8                        9                            A                         B                            C                     D                            E                            F  */
  SDL_SCANCODE_UNKNOWN,        SDL_SCANCODE_ESCAPE,         SDL_SCANCODE_1,           SDL_SCANCODE_2,              SDL_SCANCODE_3,       SDL_SCANCODE_4,              SDL_SCANCODE_5,              SDL_SCANCODE_6,          /* 0 */
  SDL_SCANCODE_7,              SDL_SCANCODE_8,              SDL_SCANCODE_9,           SDL_SCANCODE_0,              SDL_SCANCODE_MINUS,   SDL_SCANCODE_EQUALS,         SDL_SCANCODE_BACKSPACE,      SDL_SCANCODE_TAB,        /* 0 */

  SDL_SCANCODE_Q,              SDL_SCANCODE_W,              SDL_SCANCODE_E,           SDL_SCANCODE_R,              SDL_SCANCODE_T,       SDL_SCANCODE_Y,              SDL_SCANCODE_U,              SDL_SCANCODE_I,          /* 1 */
  SDL_SCANCODE_O,              SDL_SCANCODE_P,              SDL_SCANCODE_LEFTBRACKET, SDL_SCANCODE_RIGHTBRACKET,   SDL_SCANCODE_RETURN,  SDL_SCANCODE_LCTRL,          SDL_SCANCODE_A,              SDL_SCANCODE_S,          /* 1 */

  SDL_SCANCODE_D,              SDL_SCANCODE_F,              SDL_SCANCODE_G,           SDL_SCANCODE_H,              SDL_SCANCODE_J,       SDL_SCANCODE_K,              SDL_SCANCODE_L,              SDL_SCANCODE_SEMICOLON,  /* 2 */
  SDL_SCANCODE_APOSTROPHE,     SDL_SCANCODE_GRAVE,          SDL_SCANCODE_LSHIFT,      SDL_SCANCODE_BACKSLASH,      SDL_SCANCODE_Z,       SDL_SCANCODE_X,              SDL_SCANCODE_C,              SDL_SCANCODE_V,          /* 2 */

  SDL_SCANCODE_B,              SDL_SCANCODE_N,              SDL_SCANCODE_M,           SDL_SCANCODE_COMMA,          SDL_SCANCODE_PERIOD,  SDL_SCANCODE_SLASH,          SDL_SCANCODE_RSHIFT,         SDL_SCANCODE_PRINTSCREEN,/* 3 */
  SDL_SCANCODE_LALT,           SDL_SCANCODE_SPACE,          SDL_SCANCODE_CAPSLOCK,    SDL_SCANCODE_F1,             SDL_SCANCODE_F2,      SDL_SCANCODE_F3,             SDL_SCANCODE_F4,             SDL_SCANCODE_F5,         /* 3 */

  SDL_SCANCODE_F6,             SDL_SCANCODE_F7,             SDL_SCANCODE_F8,          SDL_SCANCODE_F9,             SDL_SCANCODE_F10,     SDL_SCANCODE_NUMLOCKCLEAR,   SDL_SCANCODE_SCROLLLOCK,     SDL_SCANCODE_HOME,       /* 4 */
  SDL_SCANCODE_UP,             SDL_SCANCODE_PAGEUP,         SDL_SCANCODE_KP_MINUS,    SDL_SCANCODE_LEFT,           SDL_SCANCODE_KP_5,    SDL_SCANCODE_RIGHT,          SDL_SCANCODE_KP_PLUS,        SDL_SCANCODE_END,        /* 4 */

  SDL_SCANCODE_DOWN,           SDL_SCANCODE_PAGEDOWN,       SDL_SCANCODE_INSERT,      SDL_SCANCODE_DELETE,         SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN,        SDL_SCANCODE_NONUSBACKSLASH, SDL_SCANCODE_F11,        /* 5 */
  SDL_SCANCODE_F12,            SDL_SCANCODE_PAUSE,          SDL_SCANCODE_UNKNOWN,     SDL_SCANCODE_LGUI,           SDL_SCANCODE_RGUI,    SDL_SCANCODE_APPLICATION,    SDL_SCANCODE_UNKNOWN,        SDL_SCANCODE_UNKNOWN,    /* 5 */

  SDL_SCANCODE_UNKNOWN,        SDL_SCANCODE_UNKNOWN,        SDL_SCANCODE_UNKNOWN,     SDL_SCANCODE_UNKNOWN,        SDL_SCANCODE_F13,     SDL_SCANCODE_F14,            SDL_SCANCODE_F15,            SDL_SCANCODE_F16,        /* 6 */
  SDL_SCANCODE_F17,            SDL_SCANCODE_F18,            SDL_SCANCODE_F19,         SDL_SCANCODE_UNKNOWN,        SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN,        SDL_SCANCODE_UNKNOWN,        SDL_SCANCODE_UNKNOWN,    /* 6 */

  SDL_SCANCODE_INTERNATIONAL2, SDL_SCANCODE_UNKNOWN,        SDL_SCANCODE_UNKNOWN,     SDL_SCANCODE_INTERNATIONAL1, SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN,        SDL_SCANCODE_UNKNOWN,        SDL_SCANCODE_UNKNOWN,    /* 7 */
  SDL_SCANCODE_UNKNOWN,        SDL_SCANCODE_INTERNATIONAL4, SDL_SCANCODE_UNKNOWN,     SDL_SCANCODE_INTERNATIONAL5, SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_INTERNATIONAL3, SDL_SCANCODE_UNKNOWN,        SDL_SCANCODE_UNKNOWN     /* 7 */
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

#define IS_FULLSCREEN() ((SDL_GetWindowFlags(wnd) & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0)

static inline void sys_sleep(const uint64_t us) {
    // TODO: not everything has usleep()
    usleep(us);
}

static int test_vsync(void) {
    // Even if SDL_GL_SetSwapInterval succeeds, it doesn't mean that VSync actually works.
    // A 60 Hz monitor should have a swap interval of 16.67 milliseconds.
    // Try to detect the length of a vsync by swapping buffers some times.
    // Since the graphics card may enqueue a fixed number of frames,
    // first send in four dummy frames to hopefully fill the queue.
    // This method will fail if the refresh rate is changed, which, in
    // combination with that we can't control the queue size (i.e. lag)
    // is a reason this generic SDL2 backend should only be used as last resort.

    for (int i = 0; i < 8; ++i)
        SDL_GL_SwapWindow(wnd);

    Uint32 start = SDL_GetTicks();
    SDL_GL_SwapWindow(wnd);
    SDL_GL_SwapWindow(wnd);
    SDL_GL_SwapWindow(wnd);
    SDL_GL_SwapWindow(wnd);
    Uint32 end = SDL_GetTicks();

    const float average = 4.0 * 1000.0 / (end - start);

    if (average > 27.0f && average < 33.0f) return 1;
    if (average > 57.0f && average < 63.0f) return 2;
    if (average > 86.0f && average < 94.0f) return 3;
    if (average > 115.0f && average < 125.0f) return 4;
    if (average > 234.0f && average < 246.0f) return 8;

    return 0;
}

static inline void gfx_sdl_set_vsync(const bool enabled) {
    if (enabled) {
        // try to detect refresh rate
        SDL_GL_SetSwapInterval(1);
        const int vblanks = gCLIOpts.SyncFrames ? (int)gCLIOpts.SyncFrames : test_vsync();
        if (vblanks) {
            printf("determined swap interval: %d\n", vblanks);
            SDL_GL_SetSwapInterval(vblanks);
            use_timer = false;
            return;
        } else {
            printf("could not determine swap interval, falling back to timer sync\n");
        }
    }

    use_timer = true;
    SDL_GL_SetSwapInterval(0);
}

static void gfx_sdl_set_fullscreen(void) {
    if (configWindow.reset)
        configWindow.fullscreen = false;
    if (configWindow.fullscreen == IS_FULLSCREEN())
        return;
    if (configWindow.fullscreen) {
        SDL_SetWindowFullscreen(wnd, SDL_WINDOW_FULLSCREEN_DESKTOP);
        SDL_ShowCursor(SDL_DISABLE);
    } else {
        SDL_SetWindowFullscreen(wnd, 0);
        SDL_ShowCursor(SDL_ENABLE);
        configWindow.exiting_fullscreen = true;
    }
}

static void gfx_sdl_reset_dimension_and_pos(void) {
    if (configWindow.exiting_fullscreen)
        configWindow.exiting_fullscreen = false;

    if (configWindow.reset) {
        configWindow.x = WAPI_WIN_CENTERPOS;
        configWindow.y = WAPI_WIN_CENTERPOS;
        configWindow.w = DESIRED_SCREEN_WIDTH;
        configWindow.h = DESIRED_SCREEN_HEIGHT;
        configWindow.reset = false;
    } else if (!configWindow.settings_changed) {
        return;
    }

    int xpos = (configWindow.x == WAPI_WIN_CENTERPOS) ? SDL_WINDOWPOS_CENTERED : configWindow.x;
    int ypos = (configWindow.y == WAPI_WIN_CENTERPOS) ? SDL_WINDOWPOS_CENTERED : configWindow.y;

    SDL_SetWindowSize(wnd, configWindow.w, configWindow.h);
    SDL_SetWindowPosition(wnd, xpos, ypos);
    // in case vsync changed
    gfx_sdl_set_vsync(configWindow.vsync);
}

static void gfx_sdl_init(const char *window_title) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    #ifdef USE_GLES
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);  // These attributes allow for hardware acceleration on RPis.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    #endif

    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    int xpos = (configWindow.x == WAPI_WIN_CENTERPOS) ? SDL_WINDOWPOS_CENTERED : configWindow.x;
    int ypos = (configWindow.y == WAPI_WIN_CENTERPOS) ? SDL_WINDOWPOS_CENTERED : configWindow.y;

    wnd = SDL_CreateWindow(
        window_title,
        xpos, ypos, configWindow.w, configWindow.h,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    ctx = SDL_GL_CreateContext(wnd);

    gfx_sdl_set_vsync(configWindow.vsync);

    gfx_sdl_set_fullscreen();

    perf_freq = SDL_GetPerformanceFrequency();
    frame_time = perf_freq / FRAMERATE;

    for (size_t i = 0; i < sizeof(windows_scancode_table) / sizeof(SDL_Scancode); i++) {
        inverted_scancode_table[windows_scancode_table[i]] = i;
    }

    for (size_t i = 0; i < sizeof(scancode_rmapping_extended) / sizeof(scancode_rmapping_extended[0]); i++) {
        inverted_scancode_table[scancode_rmapping_extended[i][0]] = inverted_scancode_table[scancode_rmapping_extended[i][1]] + 0x100;
    }

    for (size_t i = 0; i < sizeof(scancode_rmapping_nonextended) / sizeof(scancode_rmapping_nonextended[0]); i++) {
        inverted_scancode_table[scancode_rmapping_nonextended[i][0]] = inverted_scancode_table[scancode_rmapping_nonextended[i][1]];
        inverted_scancode_table[scancode_rmapping_nonextended[i][1]] += 0x100;
    }
}

static void gfx_sdl_main_loop(void (*run_one_game_iter)(void)) {
    run_one_game_iter();
}

static void gfx_sdl_get_dimensions(uint32_t *width, uint32_t *height) {
    int w, h;
    SDL_GetWindowSize(wnd, &w, &h);
    if (width) *width = w;
    if (height) *height = h;
}

static int translate_scancode(int scancode) {
    if (scancode < 512) {
        return inverted_scancode_table[scancode];
    } else {
        return 0;
    }
}

static void gfx_sdl_onkeydown(int scancode) {
    if (kb_key_down)
        kb_key_down(translate_scancode(scancode));

    const Uint8 *state = SDL_GetKeyboardState(NULL);

    if (state[SDL_SCANCODE_LALT] && state[SDL_SCANCODE_RETURN]) {
        configWindow.fullscreen = !configWindow.fullscreen;
        configWindow.settings_changed = true;
    }
}

static void gfx_sdl_onkeyup(int scancode) {
    if (kb_key_up)
        kb_key_up(translate_scancode(scancode));
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
            case SDL_WINDOWEVENT: // TODO: Check if this makes sense to be included in the Web build
                if (!IS_FULLSCREEN()) {
                    switch (event.window.event) {
                        case SDL_WINDOWEVENT_MOVED:
                            if (!configWindow.exiting_fullscreen) {
                                if (event.window.data1 >= 0) configWindow.x = event.window.data1;
                                if (event.window.data2 >= 0) configWindow.y = event.window.data2;
                            }
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

    if (configWindow.settings_changed) {
        gfx_sdl_set_fullscreen();
        gfx_sdl_reset_dimension_and_pos();
        configWindow.settings_changed = false;
    }
}

static void gfx_sdl_set_keyboard_callbacks(kb_callback_t on_key_down, kb_callback_t on_key_up, void (*on_all_keys_up)(void)) {
    kb_key_down = on_key_down;
    kb_key_up = on_key_up;
    kb_all_keys_up = on_all_keys_up;
}

static bool gfx_sdl_start_frame(void) {
    return true;
}

static inline void sync_framerate_with_timer(void) {
    static double last_time;
    static double last_sec;
    static int frames_since_last_sec;
    const double now = SDL_GetPerformanceCounter();
    frames_since_last_sec += 1;
    if (last_time) {
        const double elapsed = last_sec ? (now - last_sec) : (now - last_time);
        if ((elapsed < frame_time && !last_sec) || (elapsed < frames_since_last_sec * frame_time && last_sec)) {
            const double delay = last_sec ? frames_since_last_sec * frame_time - elapsed : frame_time - elapsed;
            sys_sleep(delay / perf_freq * 1000000.0);
            last_time = now + delay;
        } else {
            last_time = now;
        }
        if ((int64_t)(now / perf_freq) > (int64_t)(last_sec / perf_freq)) {
            last_sec = last_time;
            frames_since_last_sec = 0;
        }
    } else {
        last_time = now;
    }
}

static void gfx_sdl_swap_buffers_begin(void) {
    if (use_timer) sync_framerate_with_timer();
    SDL_GL_SwapWindow(wnd);
}

static void gfx_sdl_swap_buffers_end(void) {
}

static double gfx_sdl_get_time(void) {
    return 0.0;
}


static void gfx_sdl_shutdown(void) {
    if (SDL_WasInit(0)) {
        if (ctx) { SDL_GL_DeleteContext(ctx); ctx = NULL; }
        if (wnd) { SDL_DestroyWindow(wnd); wnd = NULL; }
        SDL_Quit();
    }
}

struct GfxWindowManagerAPI gfx_sdl = {
    gfx_sdl_init,
    gfx_sdl_set_keyboard_callbacks,
    gfx_sdl_main_loop,
    gfx_sdl_get_dimensions,
    gfx_sdl_handle_events,
    gfx_sdl_start_frame,
    gfx_sdl_swap_buffers_begin,
    gfx_sdl_swap_buffers_end,
    gfx_sdl_get_time,
    gfx_sdl_shutdown
};

#endif // BACKEND_WM
