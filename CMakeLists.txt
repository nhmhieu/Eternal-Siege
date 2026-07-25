cmake_minimum_required(VERSION 3.20)

project(GameProjectmoi)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System Audio)

file(GLOB_RECURSE SOURCES
    src/*.cpp
)

add_executable(GameProjectmoi
    ${SOURCES}
)

target_include_directories(GameProjectmoi PRIVATE
    include
)

target_link_libraries(GameProjectmoi PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio
)