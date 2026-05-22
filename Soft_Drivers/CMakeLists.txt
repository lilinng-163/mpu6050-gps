cmake_minimum_required(VERSION 3.22)

enable_language(C CXX)

add_library(soft_drivers OBJECT)

target_sources(soft_drivers PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/my_i2c.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/my_pwm.cpp)
target_include_directories(soft_drivers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Inc)
target_link_libraries(soft_drivers PUBLIC stm32cubemx)