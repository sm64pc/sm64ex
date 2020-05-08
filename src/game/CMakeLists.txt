cmake_minimum_required(VERSION 3.12)

project(game)


file(GLOB_RECURSE SOURCES "*.c")

file(GLOB_RECURSE HEADERS "*.h")

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})

