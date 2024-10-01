cmake_minimum_required(VERSION 3.19)

set(project_name offline_renderer)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project($project_name LANGUAGES CXX)

find_package(SDL2)

include_directories(include)

add_subdirectory(vendor)
add_subdirectory(src)
add_subdirectory(tests)
