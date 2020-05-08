cmake_minimum_required(VERSION 3.12)

project(game)

file(GLOB SOURCES "*.c")

file(GLOB HEADERS "*.h")

list(REMOVE_ITEM SOURCES ${PROJECT_SOURCE_DIR}/main.c)

list(APPEND SOURCES ${CMAKE_SOURCE_DIR}/data/behavior_data.c)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} engine)

