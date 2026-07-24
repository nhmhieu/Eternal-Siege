
#File này kiểm tra các thư viện/gcc tối thiêu cho máy mỗi khi mở source nha ae !!! 
cmake_minimum_required(VERSION 3.10)
project(RoguelikeNinja CXX)

# Ép dự án sử dụng chuẩn C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "D:/C++/SFML-3.0.2")
# Tự động tìm bộ thư viện SFML trên hệ thống (Bỏ số phiên bản cứng để nhận cả bản 2.x lẫn 3.x)
# Sửa chữ cái đầu của các component thành VIẾT HOA:
find_package(SFML COMPONENTS Graphics Window System Audio REQUIRED)

# Chỉ đường dẫn tới thư mục chứa các file Header (.hpp) của bạn
include_directories(include)

# Quét sạch tất cả file .cpp nằm trong thư mục src/
file(GLOB SOURCES "src/*.cpp")

# Tạo file thực thi (.exe) từ các file nguồn
add_executable(my_game ${SOURCES} "include/TextureManager.h" "src/TextureManager.cpp"  )

# Liên kết các module SFML vào file game của bạn (SỬA LẠI DÒNG NÀY)
target_link_libraries(my_game SFML::Graphics SFML::Window SFML::System SFML::Audio)