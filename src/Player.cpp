#include "Player.h"
#include "GameContext.h"
#include <cmath>
#include <iostream>
#include "TextureManager.h"


Player::Player(TextureManager& textureManager) {

    if (!textureManager.loadTexture("Ash", "assets/images/Ash.png")) {
        std::cerr << "Failed to load Ash texture!" << std::endl;
    }
    sprite.setTexture(textureManager.getTexture("Ash"));
    sprite.setPosition({ 400.f, 300.f });


    team = Team::Player;
    health = 100;
    maxHealth = 100;
    isAlive = true;
    speed = 300.f;

    sprite.setScale({ 0.5f, 0.5f });
}

void Player::handleInput() {
    // Di chuyển WASD
    sf::Vector2f movement(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) movement.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) movement.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) movement.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) movement.x += 1.f;

    if (movement.x != 0.f || movement.y != 0.f) {
        float length = std::sqrt(movement.x * movement.x + movement.y * movement.y);
        movement /= length;
    }
    // Lưu hướng di chuyển (dùng setter nếu có)
    setDirection(movement);

    // Tấn công bằng chuột trái
    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !isAttacking) {
        setIsAttacking(true);   // Sử dụng setter mới trong Entity.h
    }
}

void Player::update(const GameContext& context) {
    // Di chuyển theo hướng đã lưu
    sf::Vector2f dir = getDirection();
    if (dir.x != 0.f || dir.y != 0.f) {
        float newX = getX() + dir.x * speed * context.deltaTime;
        float newY = getY() + dir.y * speed * context.deltaTime;
        setPosition(newX, newY);
        sprite.setPosition(getPosition());
    }

    // Cập nhật trạng thái tấn công (tự động tắt sau attackDuration)
    updateStatus();
}

void Player::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}