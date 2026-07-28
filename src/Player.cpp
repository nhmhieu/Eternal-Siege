#include "Player.h"
#include "GameContext.h"
#include <cmath>
#include <iostream>
#include "TextureManager.h"

Player::Player(TextureManager& textureManager)
    : Entity(400.f, 300.f, 100, 100), playerTexture(nullptr) {
    team = Team::Player;

    const float desiredSize = 80.f;

    playerTexture = &textureManager.getTexture("Ash");
    playerShape.setSize(sf::Vector2f(desiredSize, desiredSize));
    playerShape.setOrigin(sf::Vector2f(desiredSize / 2.f, desiredSize / 2.f));
    playerShape.setTexture(playerTexture);
    playerShape.setPosition(sf::Vector2f(400.f, 300.f));
}


void Player::handleInput() {
    sf::Vector2f movement(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) movement.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) movement.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) movement.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) movement.x += 1.f;

    if (movement.x != 0.f || movement.y != 0.f) {
        float length = std::sqrt(movement.x * movement.x + movement.y * movement.y);
        movement /= length;
    }
    setDirection(movement);

    // Tấn công khi click chuột trái và sẵn sàng
    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && canAttack()) {
        if (!isAttacking) {
            setIsAttacking(true);
        }
    }
}

void Player::update(GameContext& context) {
    // Giảm cooldown
    if (coolDownTimer > 0.f) {
        coolDownTimer -= context.deltaTime;
        if (coolDownTimer < 0.f) coolDownTimer = 0.f;
    }

    // Xử lý input (di chuyển, tấn công)
    handleInput();

    // Di chuyển player
    sf::Vector2f dir = getDirection();
    if (dir.x != 0.f || dir.y != 0.f) {
        float newX = getX() + dir.x * speed * context.deltaTime;
        float newY = getY() + dir.y * speed * context.deltaTime;
        setPosition(newX, newY);
        playerShape.setPosition(getPosition());
    }

    // Xử lý trạng thái chết
    if (isDying) {
        updateDeadTimer(context);
    }

    // Xử lý tấn công
    if (isAttacking) {
        updateAttackTimer(context);
        if (currentWeapon) {
            currentWeapon->triggerAction(this, context, *context.combatManager);
        }
    }

    // Cập nhật trạng thái tấn công (tự tắt)
    updateStatus();
}

void Player::draw(sf::RenderWindow& window) {
    window.draw(playerShape);
    drawHealthBar(window);  // Vẽ thanh máu nếu cần
}

sf::FloatRect Player::getCollisionBox() const {
    return playerShape.getGlobalBounds();
}

sf::FloatRect Player::getHurtBox() const {
    return playerShape.getGlobalBounds();
}