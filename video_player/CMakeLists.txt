cmake_minimum_required(VERSION 3.28)
project(video_player)

set(CMAKE_CXX_STANDARD 17)
set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")
include_directories("${CMAKE_SOURCE_DIR}/include")


add_executable(video_player video_player.cpp ${SOURCE_FILES} )

add_custom_command(TARGET video_player POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${ASSETS_DIR} ${CMAKE_BINARY_DIR}/video_player/assets
)
find_package(SDL2 CONFIG REQUIRED)
find_package(FFMPEG REQUIRED)
#find_package(fmt REQUIRED)




target_link_libraries(video_player PRIVATE SDL2::SDL2)
target_include_directories(video_player  PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(video_player PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(video_player PRIVATE ${FFMPEG_LIBRARIES})
#target_link_libraries(yuv_sdl_player PRIVATE fmt::fmt-header-only)