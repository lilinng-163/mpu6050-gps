cmake_minimum_required(VERSION 3.22)

enable_language(C)

file(GLOB FreeRTOS_Sources ${CMAKE_CURRENT_SOURCE_DIR}/Src/*.c)

set(FreeRTOS_Include_Dir
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc/FreeRTOS)

add_library(freertos STATIC ${FreeRTOS_Sources})
target_include_directories(freertos PUBLIC ${FreeRTOS_Include_Dir})