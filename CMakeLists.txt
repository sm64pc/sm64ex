cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNON_MATCHING -DAVOID_UB")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNON_MATCHING -DAVOID_UB")

project(sm64pc)

set(ROM_VERSION us)
add_compile_definitions(VERSION_US)

set(HEXDUMP_PATH "C:/git-sdk-64/usr/bin/hexdump.exe")
set(N64GRAPHICS_DIR ${CMAKE_SOURCE_DIR}/tools/n64graphics)
set(SKYCONV_DIR ${CMAKE_SOURCE_DIR}/tools/skyconv)
set(MIO0_DIR ${CMAKE_SOURCE_DIR}/tools/mio0)
set(AIFC_DECODE_DIR ${CMAKE_SOURCE_DIR}/tools/aifc_decode)
set(TEXTCONV_DIR ${CMAKE_SOURCE_DIR}/tools/textconv)

IF (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
    set(N64GRAPHICS_DIR ${N64GRAPHICS_DIR}.exe)
    set(SKYCONV_DIR ${SKYCONV_DIR}.exe)
    set(MIO0_DIR ${MIO0_DIR}.exe)
    set(AIFC_DECODE_DIR ${AIFC_DECODE_DIR}.exe)
    set(AIFC_DECODE_DIR ${TEXTCONV_DIR}.exe)
    set(LINK_LIBRARIES SDL2main SDL2 glew32 opengl32 m glu32 setupapi dinput8 user32 gdi32 imm32 ole32 oleaut32 shell32 winmm version uuid)

    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(BUILD_SHARED_LIBS OFF)
    set(CMAKE_EXE_LINKER_FLAGS "-static")
ELSE ()
    set(LINK_LIBRARIES SDL2main SDL2 GL m)
ENDIF ()


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

set(SOURCES src/pc/pc_main.c)

add_executable(${PROJECT_NAME} ${SOURCES})

add_subdirectory(include)
add_subdirectory(textures)
add_subdirectory(src/audio)
add_subdirectory(src/goddard)
add_subdirectory(src/pc)
add_subdirectory(src/menu)
add_subdirectory(src/game)

target_link_libraries(${PROJECT_NAME} game audio goddard pc menu ${LINK_LIBRARIES})
