cmake_minimum_required(VERSION 3.10)
project(EternalSiege CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES "src/*.cpp")
add_executable(my_game ${SOURCES})

# === ĐƯỜNG DẪN CỨNG ===
target_include_directories(my_game PRIVATE "C:/SFML3/include" include)
target_link_directories(my_game PRIVATE "C:/SFML3/lib")

# === LINK ĐÚNG TÊN FILE (DỰA TRÊN THƯ MỤC LIB CỦA BẠN) ===
# Nếu bạn dùng Debug, link file có -d
# Nếu bạn dùng Release, link file không có -d
target_link_libraries(my_game
    "C:/SFML3/lib/sfml-graphics-d.lib"
    "C:/SFML3/lib/sfml-window-d.lib"
    "C:/SFML3/lib/sfml-system-d.lib"
)

# Copy assets
add_custom_command(TARGET my_game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:my_game>/assets
    COMMENT "Copying assets to output directory"
)