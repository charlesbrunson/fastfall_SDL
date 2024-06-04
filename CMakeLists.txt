cmake_minimum_required (VERSION 3.16)

project(ffengine)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#add_library(ffengine)
#target_compile_features(ffengine PUBLIC cxx_std_20)

add_executable(test)
target_sources(test PRIVATE main.cpp)

find_package(fmt    CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

target_link_libraries(test PRIVATE fmt::fmt)
target_link_libraries(test PRIVATE spdlog::spdlog)

#add_subdirectory(util)
