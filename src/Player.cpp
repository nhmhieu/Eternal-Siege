#include "Player.h"
#include "GameContext.h"
#include <cmath>

Player::Player() {
    this->sprite.setRadius(25.f);
    this->sprite.setFillColor(sf::Color::Cyan);
    this->sprite.setOrigin({25.f, 25.f}); // Đặt tâm ở giữa hình tròn
    this->sprite.setPosition({400.f, 300.f}); // Vị trí ban đầu
    this->speed = 300.f; // Vận tốc di chuyển (pixel/giây)
}

void Player::handleInput() {
    // Không cần xử lý trong handleInput nếu dùng Real-time Input (sf::Keyboard::isKeyPressed)
}

void Player::update(const GameContext& context) {
    // --- CƠ CHẾ 1: DI CHUYỂN BẰNG PHÍM WASD ---
    sf::Vector2f movement(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) movement.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) movement.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) movement.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) movement.x += 1.f;

    // Chuẩn hóa vector di chuyển để không bị đi chéo nhanh hơn đi thẳng
    if (movement.x != 0.f || movement.y != 0.f) {
        float length = std::sqrt(movement.x * movement.x + movement.y * movement.y);
        movement /= length;
        
        // TÍNH TOÁN VÀ CẬP NHẬT TỌA ĐỘ VÀO BIẾN LOGIC GỐC (ENTITY)
        this->x += movement.x * this->speed * context.deltaTime;
        this->y += movement.y * this->speed * context.deltaTime;
    }

    // --- CƠ CHẾ 2: DI CHUYỂN THEO CON TRỎ CHUỘT (Nếu dùng thì cần truyền window vào context) ---
    /*
    if (context.window != nullptr) { // Giả sử sau này bạn bỏ window vào struct GameContext
        sf::Vector2i mousePos = sf::Mouse::getPosition(*(context.window));
        sf::Vector2f targetPos = context.window->mapPixelToCoords(mousePos);
        
        sf::Vector2f direction = targetPos - sf::Vector2f(this->x, this->y);
        float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        
        if (distance > 5.f) { 
            direction /= distance;
            this->x += direction.x * this->speed * context.deltaTime;
            this->y += direction.y * this->speed * context.deltaTime;
        }
    }
    */

    // --- BƯỚC ĐỒNG BỘ CUỐI CÙNG: Đưa tọa độ logic ép vào Sprite hiển thị ---
    // Thay vì dùng sprite.move(), ta dùng setPosition để đảm bảo Hình ảnh và Logic khớp khít 100%
    this->sprite.setPosition(sf::Vector2f(this->x, this->y));
}

void Player::draw(sf::RenderWindow& window) {
    window.draw(this->sprite);
}