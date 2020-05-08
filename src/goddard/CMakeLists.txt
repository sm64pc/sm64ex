cmake_minimum_required(VERSION 3.12)

project(goddard)

file(GLOB_RECURSE SOURCES "*.c")

file(GLOB_RECURSE HEADERS "*.h")

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})

add_dependencies(${PROJECT_NAME} libultra textures)

target_link_libraries(${PROJECT_NAME} libultra)