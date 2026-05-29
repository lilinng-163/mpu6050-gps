cmake_minimum_required(VERSION 3.22)

enable_language(C CXX)

add_library(app OBJECT)

target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Src/app.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/Src/tim6_get.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/Src/uart_cb.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/Src/communication.cpp)
target_include_directories(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Inc)
target_link_libraries(app PUBLIC 
                        stm32cubemx 
                        freertos 
                        soft_drivers 
                        etl 
                        devices
                        cmsis-dsp)