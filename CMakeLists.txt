#File này kiểm tra các thư viện/gcc tối thiêu cho máy mỗi khi mở source nha ae !!! 
cmake_minimum_required(VERSION 3.10)
project(EternalSiege CXX)

# Ép dự án sử dụng chuẩn C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "D:/SFML-3.0.2-windows-vc17-64-bit/SFML-3.0.2")
# Tự động tìm bộ thư viện SFML trên hệ thống (Bỏ số phiên bản cứng để nhận cả bản 2.x lẫn 3.x)
# Sửa chữ cái đầu của các component thành VIẾT HOA:
find_package(SFML COMPONENTS Graphics Window System Audio REQUIRED)

# Chỉ đường dẫn tới thư mục chứa các file Header (.hpp) của bạn
include_directories(include)

# Quét sạch tất cả file .cpp nằm trong thư mục src/
file(GLOB SOURCES "src/*.cpp")
add_executable(my_game ${SOURCES})

# === ĐƯỜNG DẪN CỨNG ===
target_include_directories(my_game PRIVATE "D:/SFML-3.0.2-windows-vc17-64-bit/SFML-3.0.2/include" include)
target_link_directories(my_game PRIVATE "D:/SFML-3.0.2-windows-vc17-64-bit/SFML-3.0.2/lib")

# === LINK ĐÚNG TÊN FILE (DỰA TRÊN THƯ MỤC LIB CỦA BẠN) ===
# Nếu bạn dùng Debug, link file có -d
# Nếu bạn dùng Release, link file không có -d
target_link_libraries(my_game
    "D:/SFML-3.0.2-windows-vc17-64-bit/SFML-3.0.2/lib/sfml-window.lib"
    "D:/SFML-3.0.2-windows-vc17-64-bit/SFML-3.0.2/lib/sfml-system.lib"
    "D:/SFML-3.0.2-windows-vc17-64-bit/SFML-3.0.2/lib/sfml-graphics.lib"
)

# Copy assets
add_custom_command(TARGET my_game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:my_game>/assets
    COMMENT "Copying assets to output directory"
)