cmake_minimum_required(VERSION 3.12)

project(include)

set(GENERATED_HEADERS
        ${PROJECT_BINARY_DIR}/text_strings.h
        ${PROJECT_BINARY_DIR}/text_menu_strings.h
        )

add_custom_target(
        ${PROJECT_NAME}
        DEPENDS ${GENERATED_HEADERS}
)

add_custom_command(
        OUTPUT ${PROJECT_BINARY_DIR}/text_strings.h
        COMMAND ${TEXTCONV_DIR} charmap.txt ${PROJECT_SOURCE_DIR}/text_strings.h.in ${PROJECT_BINARY_DIR}/text_strings.h
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating text_strings.."
)

add_custom_command(
        OUTPUT ${PROJECT_BINARY_DIR}/text_menu_strings.h
        COMMAND ${TEXTCONV_DIR} charmap_menu.txt ${PROJECT_SOURCE_DIR}/text_menu_strings.h.in ${PROJECT_BINARY_DIR}/text_menu_strings.h
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating text_menu_strings.."
)