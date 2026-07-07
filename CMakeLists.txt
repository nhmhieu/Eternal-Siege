cmake_minimum_required(VERSION 3.10)
project(EternalSiege CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB SOURCES "src/*.cpp")

add_executable(my_game ${SOURCES})

target_include_directories(my_game PRIVATE "C:/SFML3/include" include)
target_link_directories(my_game PRIVATE "C:/SFML3/lib")

target_link_libraries(my_game
    "C:/SFML3/lib/sfml-graphics.lib"
    "C:/SFML3/lib/sfml-window.lib"
    "C:/SFML3/lib/sfml-system.lib"
)