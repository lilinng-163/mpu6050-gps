cmake_minimum_required(VERSION 3.22)

enable_language(C CXX)

add_library(devices OBJECT)
target_include_directories(devices PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Inc)
target_sources(devices PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Src/mpu6050.cpp)
target_link_libraries(devices PUBLIC stm32cubemx soft_drivers etl)
