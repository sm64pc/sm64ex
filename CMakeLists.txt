cmake_minimum_required(VERSION 3.12)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNON_MATCHING -DAVOID_UB")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNON_MATCHING -DAVOID_UB")

project(sm64pc)

set(ROM_VERSION us)

set(HEXDUMP_PATH "C:/git-sdk-64/usr/bin/hexdump.exe")
set(N64GRAPHICS_DIR ${CMAKE_SOURCE_DIR}/tools/n64graphics)
set(SKYCONV_DIR ${CMAKE_SOURCE_DIR}/tools/skyconv)
set(MIO0_DIR ${CMAKE_SOURCE_DIR}/tools/mio0)
set(AIFC_DECODE_DIR ${CMAKE_SOURCE_DIR}/tools/aifc_decode)

IF(WIN32)
    set(N64GRAPHICS_DIR ${N64GRAPHICS_DIR}.exe)
    set(SKYCONV_DIR ${SKYCONV_DIR}.exe)
    set(MIO0_DIR ${MIO0_DIR}.exe)
    set(AIFC_DECODE_DIR ${AIFC_DECODE_DIR}.exe)
ENDIF()

set(N64_TOOLS ${N64GRAPHICS_DIR} ${SKYCONV_DIR} ${MIO0_DIR} ${AIFC_DECODE_DIR})

include_directories(
        include
        src
        levels
        lib
        .
        ${CMAKE_BINARY_DIR}
        ${CMAKE_BINARY_DIR}/include
)

SET(HEADER_FILES src/game/memory.h)

SET(SOURCES src/pc/gfx/gfx_pc.c
            src/pc/pc_main.c
        )

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADER_FILES})

add_subdirectory(textures)
add_subdirectory(src/goddard)
add_subdirectory(src/pc)

target_link_libraries(${PROJECT_NAME} goddard pc)