cmake_minimum_required(VERSION 3.25)
project(BandHero VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SFML_STATIC_LIBRARIES FALSE)
find_package(SFML 3.0.2 REQUIRED COMPONENTS Graphics Window System Audio Network)

set(SRC_FILES src/main.cpp)
file(GLOB_RECURSE SRC_FILES "src/*.cpp")

add_executable(BandHero ${SRC_FILES})

target_include_directories(BandHero PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(BandHero PRIVATE 
    SFML::Graphics 
    SFML::Window 
    SFML::System
    SFML::Audio
    SFML::Network
)

if(MSVC)
  target_compile_options(BandHero PRIVATE /W4)
else()
  target_compile_options(BandHero PRIVATE -Wall -Wextra -Wpedantic)
endif()
