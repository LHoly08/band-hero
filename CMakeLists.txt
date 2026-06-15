cmake_minimum_required(VERSION 3.30)
project(BandHero VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
    list(APPEND CMAKE_PREFIX_PATH "C:/msys64/ucrt64")
    include_directories(SYSTEM "C:/msys64/ucrt64/include") 
endif()

set(SFML_STATIC_LIBRARIES FALSE)
find_package(SFML 3.0.2 REQUIRED COMPONENTS Graphics Window System Audio Network)

# ==============================================================================
# Dependency Management via FetchContent
# ==============================================================================
include(FetchContent)

set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE NEVER)
set(CMAKE_FETCHCONTENT_FULLY_DISCONNECTED FALSE)

# 1. Pin core ImGui to a stable release version tag to bypass the untagged docking bug
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.5-docking  # <--- Change this from 'docking' to this explicit tag
)

# 2. Keep imgui-sfml on master for SFML 3 compatibility
FetchContent_Declare(
    imgui-sfml
    GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
    GIT_TAG        master
    SYSTEM
)

# 3. Populate core ImGui
FetchContent_MakeAvailable(imgui)

# 4. Provide the path variable imgui-sfml looks for
set(IMGUI_DIR "${imgui_SOURCE_DIR}" CACHE PATH "Path to ImGui sources")

# 5. Populate imgui-sfml
FetchContent_MakeAvailable(imgui-sfml)
# ==============================================================================

file(GLOB_RECURSE SRC_FILES "src/*.cpp")

add_executable(BandHero ${SRC_FILES})

target_include_directories(BandHero PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Link everything together
target_link_libraries(BandHero PRIVATE 
    SFML::Graphics 
    SFML::Window 
    SFML::System
    SFML::Audio
    SFML::Network
    ImGui-SFML::ImGui-SFML
)

if(MSVC)
  target_compile_options(BandHero PRIVATE /W4)
else()
  target_compile_options(BandHero PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Asset deployment layout
set(ASSETS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/assets")
set(ASSETS_DEST "$<TARGET_FILE_DIR:BandHero>/assets")

add_custom_command(TARGET BandHero POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${ASSETS_SRC}"
        "${ASSETS_DEST}"
    COMMENT "Copying assets to the build output directory..."
)