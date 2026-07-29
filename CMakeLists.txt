cmake_minimum_required(VERSION 3.20)
project(EternalSiege LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_executable(my_game ${SOURCES})

target_include_directories(my_game PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
)

find_package(SFML 3 REQUIRED COMPONENTS
    Graphics
    Window
    System
)

target_link_libraries(my_game PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)

add_custom_command(
    TARGET my_game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:my_game>/assets"
)