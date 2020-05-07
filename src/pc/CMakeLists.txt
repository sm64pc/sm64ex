cmake_minimum_required(VERSION 3.12)

project(pc)

include_directories(audio controller gfx)

set(SOURCES
        pc_main.c
        configfile.c
        mixer.c
        ultra_reimplementation.c
        audio/audio_sdl.c
        audio/audio_null.c
        gfx/gfx_pc.c
        gfx/gfx_opengl.c
        gfx/gfx_sdl2.c
        controller/controller_emscripten_keyboard.c
        controller/controller_entry_point.c
        controller/controller_keyboard.c
        controller/controller_recorded_tas.c
        controller/controller_sdl.c
        )

set(HEADERS
        configfile.h
        mixer.h
        audio/audio_api.h
        audio/audio_sdl.h
        audio/audio_null.h
        gfx/gfx_pc.h
        gfx/gfx_cc.h
        gfx/gfx_opengl.h
        gfx/gfx_screen_config.h
        gfx/gfx_rendering_api.h
        gfx/gfx_sdl.h
        gfx/gfx_window_manager_api.h
        controller/controller_api.h
        controller/controller_emscripten_keyboard.h
        controller/controller_keyboard.h
        controller/controller_recorded_tas.h
        controller/controller_sdl.h
        )

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})