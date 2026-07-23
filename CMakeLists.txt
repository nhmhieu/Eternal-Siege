cmake_minimum_required(VERSION 3.10)
project(EternalSiege CXX)

# Ép dự án sử dụng chuẩn C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Quét tất cả các file .cpp nằm trong thư mục src/
file(GLOB SOURCES "src/*.cpp")

# Tạo file thực thi (.exe) từ danh sách file nguồn đã quét tự động
add_executable(my_game ${SOURCES} "include/MenuState.h" "src/MenuState.cpp" "include/IntroState.h" "src/IntroState.cpp" "include/WinState.h" "src/WinState.cpp" "src/LoseState.cpp" "include/LoseState.h")

# 1. Chỉ thẳng đường dẫn chứa thư mục include tổng của SFML và của dự án
target_include_directories(my_game PRIVATE "C:/SFML-3.0.2/include" include)

# 2. Chỉ thẳng đường dẫn chứa file thư viện liên kết (.lib)
target_link_directories(my_game PRIVATE "C:/SFML-3.0.2/lib")

# 3. Ép hệ thống nạp trực tiếp các file thư viện nền của SFML 3
target_link_libraries(my_game 
    "C:/SFML-3.0.2/lib/sfml-graphics-d.lib"
    "C:/SFML-3.0.2/lib/sfml-window-d.lib"
    "C:/SFML-3.0.2/lib/sfml-system-d.lib"
)
add_custom_command(TARGET my_game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:my_game>/assets"
)