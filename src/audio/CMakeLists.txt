cmake_minimum_required(VERSION 3.12)

project(audio)


file(GLOB SOURCES "*.c")

file(GLOB HEADERS "*.h")

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})

