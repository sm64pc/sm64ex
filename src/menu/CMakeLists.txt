cmake_minimum_required(VERSION 3.12)

project(menu)

file(GLOB SOURCES "*.c")

file(GLOB HEADERS "*.h")

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})

target_include_directories(
        ${PROJECT_NAME} PRIVATE
        ${CMAKE_BINARY_DIR}/include
)

add_dependencies(${PROJECT_NAME} include)
