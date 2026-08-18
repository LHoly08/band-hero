cmake_minimum_required(VERSION 3.30)
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


if(MSVC)
  target_compile_options(BandHero PRIVATE /W4)
else()
  target_compile_options(BandHero PRIVATE -Wall -Wextra -Wpedantic)
endif()

set(ASSETS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/assets")
set(ASSETS_DEST "$<TARGET_FILE_DIR:BandHero>/assets")

add_custom_command(TARGET BandHero POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${ASSETS_SRC}"
        "${ASSETS_DEST}"
    COMMENT "Copying assets to the build output directory..."
)
