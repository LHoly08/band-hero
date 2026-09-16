cmake_minimum_required(VERSION 3.31)
project(BandHero VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(raylib)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")

add_executable(BandHero ${SRC_FILES})

target_include_directories(BandHero PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${raylib_SOURCE_DIR}/src
)

target_link_libraries(BandHero PRIVATE raylib)

if(WIN32)
  target_compile_definitions(BandHero PRIVATE
    WIN32_LEAN_AND_MEAN
    NOGDI
    NOUSER
  )
endif()

if(MSVC)
  target_compile_options(BandHero PRIVATE /W4)
else()
  target_compile_options(BandHero PRIVATE -Wall -Wextra -Wpedantic -freflection)
endif()

set(ASSETS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/assets")
set(ASSETS_DEST "$<TARGET_FILE_DIR:BandHero>/assets")

add_custom_command(TARGET BandHero POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${ASSETS_SRC}"
        "${ASSETS_DEST}"
    COMMENT "Copying assets to the build output directory..."
)

option(BAND_HERO_BUILD_TESTS "Build headless rendering regression tests" OFF)
if(BAND_HERO_BUILD_TESTS)
  enable_testing()
  set(RENDERING_TEST_SOURCES ${SRC_FILES})
  list(FILTER RENDERING_TEST_SOURCES EXCLUDE
    REGEX "/(main|core/Game|gameplay/Settings|serial/serialib)\\.cpp$")
  add_executable(RenderingTests tests/rendering_regression.cpp ${RENDERING_TEST_SOURCES})
  # The tests supply fake GPU, input, settings and serial functions: no window,
  # device connection or settings file is needed.
  target_include_directories(RenderingTests PRIVATE include ${raylib_SOURCE_DIR}/src)
  target_compile_definitions(RenderingTests PRIVATE
    $<TARGET_PROPERTY:BandHero,COMPILE_DEFINITIONS>)
  target_compile_options(RenderingTests PRIVATE
    $<TARGET_PROPERTY:BandHero,COMPILE_OPTIONS>)
  add_custom_command(TARGET RenderingTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${ASSETS_SRC}" "$<TARGET_FILE_DIR:RenderingTests>/assets")
  add_test(NAME rendering_regression COMMAND RenderingTests)
endif()
