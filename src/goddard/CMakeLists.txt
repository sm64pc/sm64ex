cmake_minimum_required(VERSION 3.12)

project(goddard)

set(SOURCES
        debug_utils.c
        draw_objects.c
        dynlist_proc.c
        gd_main.c
        gd_math.c
        gd_memory.c
        joints.c
        objects.c
        old_menu.c
        particles.c
        renderer.c
        sfx.c
        shape_helper.c
        skin.c
        skin_movement.c
        )

set(HEADERS
        bad_declarations.h
        debug_utils.h
        draw_objects.h
        dynlist_proc.h
        gd_macros.h
        gd_main.h
        gd_math.h
        gd_memory.h
        gd_tex_dl.h
        gd_types.h
        joints.h
        objects.h
        old_menu.h
        particles.h
        renderer.h
        sfx.h
        shape_helper.h
        skin.h
        skin_movement.h
        )

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})