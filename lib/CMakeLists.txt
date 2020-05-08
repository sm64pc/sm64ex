cmake_minimum_required(VERSION 3.12)

project(libultra)

file(GLOB_RECURSE SOURCES "*.c")

file(GLOB_RECURSE HEADERS "*.h")

list(REMOVE_ITEM SOURCES
        ${PROJECT_SOURCE_DIR}/src/sqrtf.c
        ${PROJECT_SOURCE_DIR}/src/string.c
        ${PROJECT_SOURCE_DIR}/src/sprintf.c
        ${PROJECT_SOURCE_DIR}/src/_Printf.c
        ${PROJECT_SOURCE_DIR}/src/kdebugserver.c
        ${PROJECT_SOURCE_DIR}/src/osInitialize.c
        ${PROJECT_SOURCE_DIR}/src/func_802F7140.c
        ${PROJECT_SOURCE_DIR}/src/func_802F71F0.c
        ${PROJECT_SOURCE_DIR}/src/func_802F4A20.c
        ${PROJECT_SOURCE_DIR}/src/EU_D_802f4330.c
        ${PROJECT_SOURCE_DIR}/src/D_802F4380.c
        ${PROJECT_SOURCE_DIR}/src/osLeoDiskInit.c
        ${PROJECT_SOURCE_DIR}/src/osCreateThread.c
        ${PROJECT_SOURCE_DIR}/src/osDestroyThread.c
        ${PROJECT_SOURCE_DIR}/src/osStartThread.c
        ${PROJECT_SOURCE_DIR}/src/osSetThreadPri.c
        ${PROJECT_SOURCE_DIR}/src/osPiStartDma.c
        ${PROJECT_SOURCE_DIR}/src/osPiRawStartDma.c
        ${PROJECT_SOURCE_DIR}/src/osPiRawReadIo.c
        ${PROJECT_SOURCE_DIR}/src/osPiGetCmdQueue.c
        ${PROJECT_SOURCE_DIR}/src/osJamMesg.c
        ${PROJECT_SOURCE_DIR}/src/osSendMesg.c
        ${PROJECT_SOURCE_DIR}/src/osRecvMesg.c
        ${PROJECT_SOURCE_DIR}/src/osSetEventMesg.c
        ${PROJECT_SOURCE_DIR}/src/osTimer.c
        ${PROJECT_SOURCE_DIR}/src/osSetTimer.c
        ${PROJECT_SOURCE_DIR}/src/osSetTime.c
        ${PROJECT_SOURCE_DIR}/src/osCreateViManager.c
        ${PROJECT_SOURCE_DIR}/src/osViSetSpecialFeatures.c
        ${PROJECT_SOURCE_DIR}/src/osVirtualToPhysical.c
        ${PROJECT_SOURCE_DIR}/src/osViBlack.c
        ${PROJECT_SOURCE_DIR}/src/osViSetEvent.c
        ${PROJECT_SOURCE_DIR}/src/osViSetMode.c
        ${PROJECT_SOURCE_DIR}/src/osViSwapBuffer.c
        ${PROJECT_SOURCE_DIR}/src/osSpTaskLoadGo.c
        ${PROJECT_SOURCE_DIR}/src/osCreatePiManager.c
        ${PROJECT_SOURCE_DIR}/src/osGetTime.c
        ${PROJECT_SOURCE_DIR}/src/osEepromProbe.c
        ${PROJECT_SOURCE_DIR}/src/osEepromWrite.c
        ${PROJECT_SOURCE_DIR}/src/osEepromLongWrite.c
        ${PROJECT_SOURCE_DIR}/src/osEepromRead.c
        ${PROJECT_SOURCE_DIR}/src/osEepromLongRead.c
        ${PROJECT_SOURCE_DIR}/src/osContInit.c
        ${PROJECT_SOURCE_DIR}/src/osContStartReadData.c
        ${PROJECT_SOURCE_DIR}/src/osAiGetLength.c
        ${PROJECT_SOURCE_DIR}/src/osAiSetFrequency.c
        ${PROJECT_SOURCE_DIR}/src/osAiSetNextBuffer.c
        ${PROJECT_SOURCE_DIR}/src/__osViInit.c
        ${PROJECT_SOURCE_DIR}/src/__osSyncPutChars.c
        ${PROJECT_SOURCE_DIR}/src/__osAtomicDec.c
        ${PROJECT_SOURCE_DIR}/src/__osSiRawStartDma.c
        ${PROJECT_SOURCE_DIR}/src/__osViSwapContext.c
        ${PROJECT_SOURCE_DIR}/src/__osViGetCurrentContext.c
        ${PROJECT_SOURCE_DIR}/src/__osDevMgrMain.c
        )

list(APPEND SOURCES
        ${CMAKE_SOURCE_DIR}/src/pc/ultra_reimplementation.c
        ${CMAKE_SOURCE_DIR}/src/pc/mixer.c
        )

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
