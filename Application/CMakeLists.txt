cmake_minimum_required(VERSION 3.22)

enable_language(C CXX)

add_library(app OBJECT)

target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Src/app.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/Src/tim6_get.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/Src/uart_cb.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/Src/communication.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/Src/task_iwdg.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/Src/task_cpu_stats.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/Src/task_led.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/Src/task_servo.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/Src/task_mpu6050_collect.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/Src/task_mpu6050_calculate.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/Src/task_oled.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/Src/task_gps.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/Src/task_cmt.cpp)
target_include_directories(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Inc)
target_link_libraries(app PUBLIC 
                        stm32cubemx 
                        freertos 
                        soft_drivers 
                        etl 
                        devices
                        cmsis-dsp)