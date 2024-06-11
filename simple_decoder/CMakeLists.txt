cmake_minimum_required(VERSION 3.28)
project(simple_decoder)

set(CMAKE_CXX_STANDARD 17)
set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")
#set(CMAKE_TOOLCHAIN_FILE "/Users/miuhaochu/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
#set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/vcpkg_installed/arm64-osx")
find_package(FFMPEG REQUIRED)
#add_custom_command(TARGET simple_decoder PRE_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_directory
#        ${CMAKE_SOURCE_DIR}/assets)

add_executable(simple_decoder simple_decoder.cpp)

add_custom_command(TARGET simple_decoder POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${ASSETS_DIR} ${CMAKE_BINARY_DIR}/simple_decoder/assets
)
target_include_directories(simple_decoder  PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(simple_decoder PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries( simple_decoder PRIVATE ${FFMPEG_LIBRARIES})
