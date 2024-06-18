cmake_minimum_required(VERSION 3.28)
project(ffplay_refactor)

set(CMAKE_CXX_STANDARD 17)
set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")

#enable_testing()


include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(ffplay_refactor main.cpp ${SOURCE_FILES})

add_executable(ffplay_test fifo_test.cpp ${SOURCE_FILES})

add_custom_command(TARGET ffplay_refactor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${ASSETS_DIR} ${CMAKE_BINARY_DIR}/ffplay_refactor/assets
)
find_package(SDL2 CONFIG REQUIRED)
find_package(FFMPEG REQUIRED)
find_package(fmt REQUIRED)
find_package(GTest REQUIRED)




target_link_libraries(ffplay_refactor PRIVATE SDL2::SDL2)

target_include_directories(ffplay_refactor  PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(ffplay_refactor PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(ffplay_refactor PRIVATE ${FFMPEG_LIBRARIES})
target_link_libraries(ffplay_refactor PRIVATE fmt::fmt-header-only)




target_link_libraries(ffplay_test PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
target_link_libraries(ffplay_test PRIVATE SDL2::SDL2)

target_include_directories(ffplay_test  PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(ffplay_test PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(ffplay_test PRIVATE ${FFMPEG_LIBRARIES})
target_link_libraries(ffplay_test PRIVATE fmt::fmt-header-only)
add_test(AllTestsInMain ffplay_test)