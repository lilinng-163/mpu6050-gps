cmake_minimum_required(VERSION 3.22)

enable_language(C CXX)

add_library(etl INTERFACE)
target_include_directories(etl INTERFACE D:/lib/mcu/C++/etl/include)
