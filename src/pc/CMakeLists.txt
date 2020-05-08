cmake_minimum_required(VERSION 3.12)

project(pc)

include_directories(audio controller gfx)

file(GLOB_RECURSE SOURCES "*.c")

file(GLOB_RECURSE HEADERS "*.h")

list(REMOVE_ITEM SOURCES ${PROJECT_SOURCE_DIR}/pc_main.c)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
