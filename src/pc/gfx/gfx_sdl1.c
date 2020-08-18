#ifdef WAPI_SDL1

#ifdef __MINGW32__
#define FOR_WINDOWS 1
#else
#define FOR_WINDOWS 0
#endif

#include <SDL/SDL.h>

#include <stdio.h>
#include <unistd.h>

#include "gfx_window_manager_api.h"
#include "gfx_screen_config.h"
#include "../pc_main.h"
#include "../configfile.h"
#include "../cliopts.h"
#include "../platform.h"

#include "src/pc/controller/controller_keyboard.h"

// TODO: figure out if this shit even works
#ifdef VERSION_EU
# define FRAMERATE 25
#else
# define FRAMERATE 30
#endif

static int inverted_scancode_table[512];

static kb_callback_t kb_key_down = NULL;
static kb_callback_t kb_key_up = NULL;
static void (*kb_all_keys_up)(void) = NULL;

// time between consequtive game frames
static const int frame_time = 1000 / FRAMERATE;

static int desktop_w = 640;
static int desktop_h = 480;
static int desktop_bpp = 24;

static int window_w = 0;
static int window_h = 0;

const SDLKey windows_scancode_table[] = {
    /*  0            1               2                 3                  4             5                  6                    7                     */
    /*  8            9               A                 B                  C             D                  E                    F                     */
    SDLK_UNKNOWN,    SDLK_ESCAPE,    SDLK_1,           SDLK_2,            SDLK_3,       SDLK_4,            SDLK_5,              SDLK_6,          /* 0 */
    SDLK_7,          SDLK_8,         SDLK_9,           SDLK_0,            SDLK_MINUS,   SDLK_EQUALS,       SDLK_BACKSPACE,      SDLK_TAB,        /* 0 */

    SDLK_q,          SDLK_w,         SDLK_e,           SDLK_r,            SDLK_t,       SDLK_y,            SDLK_u,              SDLK_i,          /* 1 */
    SDLK_o,          SDLK_p,         SDLK_LEFTBRACKET, SDLK_RIGHTBRACKET, SDLK_RETURN,  SDLK_LCTRL,        SDLK_a,              SDLK_s,          /* 1 */

    SDLK_d,          SDLK_f,         SDLK_g,           SDLK_h,            SDLK_j,       SDLK_k,            SDLK_l,              SDLK_SEMICOLON,  /* 2 */
    SDLK_UNKNOWN,    SDLK_BACKQUOTE, SDLK_LSHIFT,      SDLK_BACKSLASH,    SDLK_z,       SDLK_x,            SDLK_c,              SDLK_v,          /* 2 */

    SDLK_b,          SDLK_n,         SDLK_m,           SDLK_COMMA,        SDLK_PERIOD,  SDLK_SLASH,        SDLK_RSHIFT,         SDLK_PRINT,      /* 3 */
    SDLK_LALT,       SDLK_SPACE,     SDLK_CAPSLOCK,    SDLK_F1,           SDLK_F2,      SDLK_F3,           SDLK_F4,             SDLK_F5,         /* 3 */

    SDLK_F6,         SDLK_F7,        SDLK_F8,          SDLK_F9,           SDLK_F10,     SDLK_NUMLOCK,      SDLK_SCROLLOCK,      SDLK_HOME,       /* 4 */
    SDLK_UP,         SDLK_PAGEUP,    SDLK_KP_MINUS,    SDLK_LEFT,         SDLK_KP5,     SDLK_RIGHT,        SDLK_KP_PLUS,        SDLK_END,        /* 4 */

    SDLK_DOWN,       SDLK_PAGEDOWN,  SDLK_INSERT,      SDLK_DELETE,       SDLK_UNKNOWN, SDLK_UNKNOWN,      SDLK_UNKNOWN,        SDLK_F11,        /* 5 */
    SDLK_F12,        SDLK_PAUSE,     SDLK_UNKNOWN,     SDLK_LSUPER,       SDLK_RSUPER,  SDLK_MODE,         SDLK_UNKNOWN,        SDLK_UNKNOWN,    /* 5 */

    SDLK_UNKNOWN,    SDLK_UNKNOWN,   SDLK_UNKNOWN,     SDLK_UNKNOWN,      SDLK_F13,     SDLK_F14,          SDLK_F15,            SDLK_UNKNOWN,    /* 6 */
    SDLK_UNKNOWN,    SDLK_UNKNOWN,   SDLK_UNKNOWN,     SDLK_UNKNOWN,      SDLK_UNKNOWN, SDLK_UNKNOWN,      SDLK_UNKNOWN,        SDLK_UNKNOWN,    /* 6 */

    SDLK_WORLD_2,    SDLK_UNKNOWN,   SDLK_UNKNOWN,     SDLK_WORLD_1,      SDLK_UNKNOWN, SDLK_UNKNOWN,      SDLK_UNKNOWN,        SDLK_UNKNOWN,    /* 7 */
    SDLK_UNKNOWN,    SDLK_WORLD_4,   SDLK_UNKNOWN,     SDLK_WORLD_5,      SDLK_UNKNOWN, SDLK_WORLD_3,      SDLK_UNKNOWN,        SDLK_UNKNOWN     /* 7 */
};

const SDLKey scancode_rmapping_extended[][2] = {
    { SDLK_KP_ENTER,  SDLK_RETURN },
    { SDLK_RALT,      SDLK_LALT },
    { SDLK_RCTRL,     SDLK_LCTRL },
    { SDLK_KP_DIVIDE, SDLK_SLASH },
    //{ SDLK_KPPLUS,    SDLK_CAPSLOCK }
};

const SDLKey scancode_rmapping_nonextended[][2] = {
    { SDLK_KP7,         SDLK_HOME },
    { SDLK_KP8,         SDLK_UP },
    { SDLK_KP9,         SDLK_PAGEUP },
    { SDLK_KP4,         SDLK_LEFT },
    { SDLK_KP6,         SDLK_RIGHT },
    { SDLK_KP1,         SDLK_END },
    { SDLK_KP2,         SDLK_DOWN },
    { SDLK_KP3,         SDLK_PAGEDOWN },
    { SDLK_KP0,         SDLK_INSERT },
    { SDLK_KP_PERIOD,   SDLK_DELETE },
    { SDLK_KP_MULTIPLY, SDLK_PRINT }
};

static void gfx_sdl_set_mode(void) {
    if (configWindow.exiting_fullscreen)
        configWindow.exiting_fullscreen = false;

    if (configWindow.reset) {
        configWindow.fullscreen = false;
        configWindow.x = WAPI_WIN_CENTERPOS;
        configWindow.y = WAPI_WIN_CENTERPOS;
        configWindow.w = DESIRED_SCREEN_WIDTH;
        configWindow.h = DESIRED_SCREEN_HEIGHT;
        configWindow.reset = false;
    }

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    uint32_t flags = SDL_OPENGL;
    if (configWindow.fullscreen)
        flags |= SDL_FULLSCREEN;
    else
        flags |= SDL_RESIZABLE;

    if (!SDL_VideoModeOK(configWindow.w, configWindow.h, desktop_bpp, flags)) {
        printf(
            "video mode [%dx%d fullscreen %d] not available, falling back to default\n",
            configWindow.w, configWindow.h, configWindow.fullscreen
        );
        configWindow.w = DESIRED_SCREEN_WIDTH;
        configWindow.h = DESIRED_SCREEN_HEIGHT;
        configWindow.fullscreen = false;
        flags = SDL_OPENGL | SDL_RESIZABLE;
    }

    if (!SDL_SetVideoMode(configWindow.w, configWindow.h, desktop_bpp, flags)) {
        sys_fatal(
            "could not set video mode [%dx%d fullscreen %d]: %s\n",
            configWindow.w, configWindow.h, configWindow.fullscreen, SDL_GetError()
        );
    }

    window_w = configWindow.w;
    window_h = configWindow.h;
}

static void gfx_sdl_init(const char *window_title) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
        sys_fatal("Could not init SDL1 video: %s\n", SDL_GetError());

    const SDL_VideoInfo *vinfo = SDL_GetVideoInfo();
    desktop_w = vinfo->current_w;
    desktop_h = vinfo->current_h;
    desktop_bpp = vinfo->vfmt->BitsPerPixel;

    SDL_WM_SetCaption(window_title, NULL);

    // set actual desired video mode

    gfx_sdl_set_mode();

    SDL_ShowCursor(0);

    for (size_t i = 0; i < sizeof(windows_scancode_table) / sizeof(SDLKey); i++) {
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
    if (width) *width = window_w;
    if (height) *height = window_h;
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
                gfx_sdl_onkeydown(event.key.keysym.sym);
                // ALT+F4 in case the OS doesn't do it (SDL1 doesn't seem to do it on my machine)
                if (event.key.keysym.sym == SDLK_F4 && (event.key.keysym.mod & (KMOD_LALT | KMOD_RALT)))
                    game_exit();
                break;
            case SDL_KEYUP:
                gfx_sdl_onkeyup(event.key.keysym.sym);
                break;
#endif
            case SDL_VIDEORESIZE:
                window_w = configWindow.w = event.resize.w;
                window_h = configWindow.h = event.resize.h;
                break;
            case SDL_QUIT:
                game_exit();
                break;
        }
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
    static Uint32 last_time = 0;
    // get base timestamp on the first frame (might be different from 0)
    if (last_time == 0) last_time = SDL_GetTicks();
    const int elapsed = SDL_GetTicks() - last_time;
    if (elapsed < frame_time)
        SDL_Delay(frame_time - elapsed);
    last_time += frame_time;
}

static void gfx_sdl_swap_buffers_begin(void) {
    sync_framerate_with_timer();
    SDL_GL_SwapBuffers();
}

static void gfx_sdl_swap_buffers_end(void) {
}

static double gfx_sdl_get_time(void) {
    return 0.0;
}


static void gfx_sdl_shutdown(void) {
    if (SDL_WasInit(0))
        SDL_Quit();
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
