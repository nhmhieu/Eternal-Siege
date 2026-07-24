#include "Player.h"
#include <cmath>

//Player::Player() {
//    this->sprite.setRadius(25.f);
//    this->sprite.setFillColor(sf::Color::Cyan);
//    this->sprite.setOrigin({25.f, 25.f}); // Đặt tâm ở giữa hình tròn
//    this->sprite.setPosition({400.f, 300.f}); // Vị trí ban đầu
//    this->speed = 300.f; // Vận tốc di chuyển (pixel/giây)
//}
Player::Player() {
    // Lấy reference tới texture từ TextureManager
    const sf::Texture& texture = TextureManager::getInstance().getTexture("player");

    // Khởi tạo con trỏ unique_ptr<sf::Sprite> truyền vào texture
    this->sprite = std::make_unique<sf::Sprite>(texture);

    // Tính toán và thiết lập origin ở trung tâm ảnh
    sf::Vector2u size = texture.getSize();
    this->sprite->setOrigin({ static_cast<float>(size.x) / 2.f, static_cast<float>(size.y) / 2.f });

    // Đặt vị trí ban đầu và tốc độ
    this->sprite->setPosition({ 400.f, 300.f });
    this->speed = 300.f;
}

void Player::handleInput() {
    // Không cần xử lý trong handleInput nếu dùng Real-time Input (sf::Keyboard::isKeyPressed)
}

void Player::update(float dt, const sf::RenderWindow& window) {
    // --- CƠ CHẾ 1: DI CHUYỂN BẰNG PHÍM WASD ---
    // sf::Vector2f movement(0.f, 0.f);
    // if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) movement.y -= 1.f;
    // if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) movement.y += 1.f;
    // if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) movement.x -= 1.f;
    // if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) movement.x += 1.f;

    // // Chuẩn hóa vector di chuyển để không bị đi chéo nhanh hơn đi thẳng
    // if (movement.x != 0.f || movement.y != 0.f) {
    //     // Tự tính toán chuẩn hóa độ dài vector thủ công (Hạn chế dùng hàm thư viện ngoài)
    //     float length = std::sqrt(movement.x * movement.x + movement.y * movement.y);
    //     movement /= length;
    //     this->sprite.move(movement * this->speed * dt);
    // }

    // --- CƠ CHẾ 2: DI CHUYỂN THEO CON TRỎ CHUỘT (Bỏ comment đoạn dưới nếu muốn dùng) ---
    
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f targetPos = window.mapPixelToCoords(mousePos);
    sf::Vector2f playerPos = this->sprite->getPosition();
    
    sf::Vector2f direction = targetPos - playerPos;
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    
    if (distance > 5.f) { // Tránh hiện tượng rung lắc khi đến sát con trỏ
        direction /= distance;
        this->sprite->move(direction * this->speed * dt);
    }
    
}

void Player::render(sf::RenderWindow& window) {
    //window.draw(this->sprite);
    window.draw(*this->sprite);
}