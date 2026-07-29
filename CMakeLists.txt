cmake_minimum_required(VERSION 3.20)

project(EternalSiege LANGUAGES CXX)

# ==============================
# C++17
# ==============================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================
# SFML
# ==============================
set(CMAKE_PREFIX_PATH
    "D:/SFML-3.0.2-windows-vc17-64-bit/SFML-3.0.2"
)

find_package(SFML 3 COMPONENTS
    Graphics
    Window
    System
    Audio
    REQUIRED
)

# ==============================
# Source files
# ==============================
file(GLOB SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

# ==============================
# Create executable
# ==============================
add_executable(my_game ${SOURCES})

# ==============================
# Header files
# ==============================
target_include_directories(my_game PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
)

# ==============================
# Link SFML
# ==============================
target_link_libraries(my_game PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio
)

# ==============================
# Đưa .exe vào chính thư mục
# x64-Debug
# ==============================
set_target_properties(my_game PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}"
)

# ==============================
# Copy assets
# ==============================
add_custom_command(TARGET my_game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:my_game>/assets"
    COMMENT "Copying assets to output directory"
)