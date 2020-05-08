cmake_minimum_required(VERSION 3.12)

project(textures)

add_custom_target(
        extract_assets
)

add_custom_command(
        TARGET extract_assets
        COMMAND python ${CMAKE_SOURCE_DIR}/extract_assets.py ${ROM_VERSION}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        DEPENDS ${N64_TOOLS}
        COMMENT "Extracting assets from: baserom.${ROM_VERSION}.z64"
)

add_custom_target(
        clean_textures
)

add_custom_command(
        TARGET clean_textures
        COMMAND python ${CMAKE_SOURCE_DIR}/extract_assets.py --clean
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        DEPENDS ${N64_TOOLS}
        COMMENT "Cleaning textures.."
)


file(GLOB_RECURSE TEXTURE_PNGS "${PROJECT_SOURCE_DIR}/*.*.png")
set(OUTPUT_DUMPED)
set(OUTPUT_HEX)

foreach(TEXTURE_PNG ${TEXTURE_PNGS})
    file(RELATIVE_PATH TEXTURE_REL_PATH ${PROJECT_SOURCE_DIR} ${TEXTURE_PNG})
    get_filename_component(TEXTURE_DUMPED ${TEXTURE_REL_PATH} NAME_WLE)
    get_filename_component(REL_PATH ${TEXTURE_REL_PATH} DIRECTORY)
    get_filename_component(TEXTURE_FORMAT_TEMP ${TEXTURE_DUMPED} LAST_EXT)
    string(SUBSTRING ${TEXTURE_FORMAT_TEMP} 1 -1 TEXTURE_FORMAT)
    file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/${REL_PATH})
    add_custom_command(
            OUTPUT ${PROJECT_BINARY_DIR}/${REL_PATH}/${TEXTURE_DUMPED}
            COMMAND ${N64GRAPHICS_DIR} -i ${PROJECT_BINARY_DIR}/${REL_PATH}/${TEXTURE_DUMPED} -g ${PROJECT_SOURCE_DIR}/${TEXTURE_REL_PATH} -f ${TEXTURE_FORMAT}
            COMMENT "extracting ${PROJECT_SOURCE_DIR}/${TEXTURE_REL_PATH} to ${PROJECT_BINARY_DIR}/${REL_PATH}/${TEXTURE_DUMPED}"
            DEPENDS ${PROJECT_SOURCE_DIR}/${TEXTURE_REL_PATH}
    )
    list(APPEND OUTPUT_DUMPED ${PROJECT_BINARY_DIR}/${REL_PATH}/${TEXTURE_DUMPED})
    set(TEXTURE_HEX ${TEXTURE_DUMPED}.inc.c)
    add_custom_command(
            OUTPUT ${PROJECT_BINARY_DIR}/${REL_PATH}/${TEXTURE_HEX}
            COMMAND ${HEXDUMP_PATH} -v -e '1/1 \"0x%%X,\"' ${PROJECT_BINARY_DIR}/${REL_PATH}/${TEXTURE_DUMPED} > ${PROJECT_BINARY_DIR}/${REL_PATH}/${TEXTURE_HEX}
            COMMENT "dumping ${PROJECT_BINARY_DIR}/${REL_PATH}/${TEXTURE_DUMPED} to ${PROJECT_BINARY_DIR}/${REL_PATH}/${TEXTURE_HEX}"
            DEPENDS ${PROJECT_BINARY_DIR}/${REL_PATH}/${TEXTURE_DUMPED}
    )
    list(APPEND OUTPUT_HEX ${PROJECT_BINARY_DIR}/${REL_PATH}/${TEXTURE_HEX})
endforeach()

add_custom_target(
        ${PROJECT_NAME}
        DEPENDS extract_assets ${OUTPUT_DUMPED} ${OUTPUT_HEX}
)
