cmake_minimum_required(VERSION 3.28)
project(simple_sdl)

set(CMAKE_CXX_STANDARD 17)

add_executable(simple_sdl_main simple_sdl.cpp)

find_package(SDL2 CONFIG REQUIRED)


target_link_libraries(simple_sdl_main PRIVATE SDL2::SDL2)