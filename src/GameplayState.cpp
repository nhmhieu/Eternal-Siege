#include "GameplayState.h"
#include <iostream>

void GameplayState::onEnter() {
    std::cout << "Giai doan: Buoc vao man choi Gameplay!\n";
}

void GameplayState::onExit() {
    std::cout << "Giai doan: Thoat khoi man choi Gameplay!\n";
}

void GameplayState::handleEvent(const sf::Event& event) {
    // Xử lý các sự kiện dạng một lần (như nhấn nút Pause, mở Inventory) theo chuẩn SFML 3
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::P) {
            std::cout << "Tam dung Game (Pause)!\n";
            // Bạn có thể push State Pause vào StateMachine ở đây nếu cần
        }
    }
}

void GameplayState::update(float dt) {
    // Không cần truyền window nếu dùng WASD, nhưng cần nếu dùng cơ chế di chuyển theo chuột
    // Ở đây tạm thời để trống hoặc bạn có thể chỉnh sửa lại tham số tùy ý.
    // Để lấy được tương tác chuột chính xác, ta cập nhật player dựa trên trạng thái phím/chuột.
}

// Hàm update cải tiến nhận thêm tham chiếu window để phục vụ việc lấy tọa độ chuột
void GameplayState::render(sf::RenderWindow& window) {
    // Cập nhật Player ngay trong vòng lặp chính (Dùng delta time đếm ngược từ Clock của Game)
    // Lưu ý: Thông thường hàm update nhân vật sẽ nằm ở hàm update(dt), bạn có thể lưu con trỏ window 
    // hoặc truyền thẳng window vào hàm render này để cập nhật tọa độ chuột trước khi vẽ.
    
    // Ví dụ cập nhật nhanh tọa độ trực tiếp:
    this->player.update(0.016f, window); // Giả lập dt = 1/60s nếu chưa truyền trực tiếp qua hệ thống
    
    // Vẽ nhân vật
    this->player.render(window);
}