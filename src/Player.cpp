#include "Player.h"
#include "GameContext.h"

#include <iostream>
#include <cmath>

using namespace std ;


Player::Player() {
    this->sprite.setRadius(25.f);
    this->sprite.setFillColor(sf::Color::Cyan);
    this->sprite.setOrigin({25.f, 25.f}); // Đặt tâm ở giữa hình tròn
    this->sprite.setPosition({400.f, 300.f}); // Vị trí ban đầu
    this->speed = 300.f; // Vận tốc di chuyển (pixel/giây)
}

void Player::handleInput() {
    // handleInput LÀM NHIỆM VỤ ĐỌC PHÍM (Ý định của người chơi)
    sf::Vector2f movement(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) movement.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) movement.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) movement.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) movement.x += 1.f;

    // Chuẩn hóa vector di chuyển
    if (movement.x != 0.f || movement.y != 0.f) {
        float length = std::sqrt(movement.x * movement.x + movement.y * movement.y);
        movement /= length;
    }
    
    // Lưu kết quả vào biến direction của Entity để update dùng
    this->direction = movement; 

    // --- 2. XỬ LÝ TẤN CÔNG (THÊM ĐOẠN NÀY VÀO) ---
    // Kiểm tra: Nếu chuột trái ĐANG ĐƯỢC BẤM và hiện tại ĐANG KHÔNG TẤN CÔNG
    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !this->isAttacking) {
        this->isAttacking = true;
        this->attackClock.restart(); // Bấm giờ để bắt đầu tính thời gian đòn đánh tồn tại
        
        std::cout << "Player vung don tan cong!\n"; // Dòng in ra để bạn test xem bấm chuột ăn chưa
    }

}

void Player::update(const GameContext& context) {
   
    if (this->direction.x != 0.f || this->direction.y != 0.f) {
        this->x += this->direction.x * this->speed * context.deltaTime;
        this->y += this->direction.y * this->speed * context.deltaTime;
    }

    //Dong bo vi tri vao sprite 
    this->sprite.setPosition(sf::Vector2f(this->x, this->y));
    this->updateStatus() ;
    
}

void Player::draw(sf::RenderWindow& window) {
    window.draw(this->sprite);


    // 2. Kiểm tra điều kiện: Nếu đang tấn công thì vẽ ô Hitbox Debug
    // if (this->isAttacking) {
    //     // Gọi hàm toán học lấy vùng FloatRect (Chuẩn SFML 3.0) mà bạn đã làm
    //     sf::FloatRect attackBox = this->getAttackHitbox();

    //     // Tạo hình chữ nhật trực quan để hiển thị
    //     sf::RectangleShape debugRect;
    //     debugRect.setPosition(attackBox.position);
    //     debugRect.setSize(attackBox.size);

    //     // Định dạng màu sắc để dễ nhìn thấy trên màn hình
    //     debugRect.setFillColor(sf::Color(255, 0, 0, 100)); // Màu đỏ bán trong suốt (Alpha = 100)
    //     debugRect.setOutlineColor(sf::Color::Red);         // Viền đỏ đậm
    //     debugRect.setOutlineThickness(1.2f);               // Độ dày viền

    //     // Vẽ cái ô này lên cửa sổ
    //     window.draw(debugRect);

    //     //Kiem tra huong cua don danh
    //     cout << "Attack direction : " << this->attackDirection.x << " " << this->attackDirection.y << endl ; 
    // }
}