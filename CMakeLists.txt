cmake_minimum_required(VERSION 3.28)
project(ffm)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/vcpkg_installed/arm64-osx")
include_directories("${CMAKE_SOURCE_DIR}/include")

file(GLOB SOURCE_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)

#find_package(fmt CONFIG REQUIRED)
#find_package(FFMPEG REQUIRED)
#find_package(SDL2 CONFIG REQUIRED)
if(ENABLE_ASAN)
    message(STATUS "build with ASAN")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
endif ()


add_subdirectory(simple_decoder)
add_subdirectory(simple_sdl)
add_subdirectory(yuv_sdl_player)
add_subdirectory(video_player)

#target_include_directories(ffm  PRIVATE ${FFMPEG_INCLUDE_DIRS})
#target_link_libraries(ffm PRIVATE fmt::fmt)
#target_link_directories(ffm PRIVATE ${FFMPEG_LIBRARY_DIRS})
#target_link_libraries(ffm PRIVATE ${FFMPEG_LIBRARIES})


