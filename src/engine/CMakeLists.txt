cmake_minimum_required(VERSION 3.12)

project(engine)

file(GLOB SOURCES "*.c")

file(GLOB HEADERS "*.h")

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} menu game)
