cmake_minimum_required(VERSION 3.15)
project(SFMLGame VERSION 1.0.0 LANGUAGES CXX)

# Sử dụng chuẩn C++17 trở lên (SFML 3 bắt buộc tối thiểu C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SỬA TẠI ĐÂY: Viết hoa chữ cái đầu của các COMPONENTS cho SFML 3
find_package(SFML 3 COMPONENTS Graphics Window System Audio REQUIRED)

# Khai báo đường dẫn chứa các file header (.h)
include_directories(include)

# Tự động tìm tất cả file .cpp trong thư mục src/
file(GLOB_RECURSE SRC_FILES "src/*.cpp")

# Khởi tạo file thực thi đầu ra
# Khởi tạo file thực thi đầu ra (đã sửa đường dẫn thành src/main.cpp)
add_executable(${PROJECT_NAME} src/main.cpp ${SRC_FILES}
    include/AttackComponent.h)

# SỬA TẠI ĐÂY: Liên kết target theo chuẩn namespace của SFML 3 (SFML::<Component>)
target_link_libraries(${PROJECT_NAME} SFML::Graphics SFML::Window SFML::System SFML::Audio)