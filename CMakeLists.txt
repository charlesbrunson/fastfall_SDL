cmake_minimum_required (VERSION 3.22)

project(ffengine)

#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_EXTENSIONS OFF)
#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(ffengine)

find_package(fmt    CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

find_path(RAPIDXML_INCLUDE_DIRS "rapidxml/rapidxml.hpp")
target_include_directories(ffengine PRIVATE ${RAPIDXML_INCLUDE_DIRS})

target_link_libraries(ffengine PRIVATE fmt::fmt)
target_link_libraries(ffengine PRIVATE spdlog::spdlog)

target_compile_features(ffengine PUBLIC cxx_std_20)
target_include_directories(ffengine PUBLIC  "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_subdirectory(src)