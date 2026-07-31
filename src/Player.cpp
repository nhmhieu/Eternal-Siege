#include "Player.h"
#include "BalanceConfig.h"
#include "GameContext.h"
#include <cmath>
#include <iostream>
#include "TextureManager.h"
#include "Constants.h"
#include "Map.h"
#include <algorithm>

Player::Player(TextureManager& textureManager)
    : Entity(
          400.f,
          300.f,
          BalanceConfig::PLAYER_MAX_HEALTH,
          BalanceConfig::PLAYER_MAX_HEALTH
      ),
      playerTexture(nullptr) {
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

}

void Player::setPosition(const sf::Vector2f& pos) {
    Entity::setPosition(pos.x, pos.y);
    playerShape.setPosition(pos);
}

void Player::update(GameContext& context) {
    Entity::update(context);

    if (isDead()) {
        updateDeadTimer(context);
        return;
    }

    // X? l input (di chuy?n, t?n cng)
    handleInput();

    // Di chuy?n player
    sf::Vector2f dir = getDirection();
    if (dir.x != 0.f || dir.y != 0.f) {
        constexpr float collisionHalfSize = 18.f;
        sf::Vector2f nextPosition = getPosition();

        const sf::Vector2f horizontal{
            nextPosition.x + dir.x * speed * context.deltaTime,
            nextPosition.y
        };
        if (!context.map ||
            context.map->isWalkableWorld(horizontal, collisionHalfSize)) {
            nextPosition.x = horizontal.x;
        }

        const sf::Vector2f vertical{
            nextPosition.x,
            nextPosition.y + dir.y * speed * context.deltaTime
        };
        if (!context.map ||
            context.map->isWalkableWorld(vertical, collisionHalfSize)) {
            nextPosition.y = vertical.y;
        }

        setPosition(nextPosition);
    }

    // X? l t?n cng
    if (isAttacking) {
        updateAttackTimer(context);
        if (currentWeapon && context.combatManager) {
            currentWeapon->triggerAction(this, context, *context.combatManager);
        }
    }

    // C?p nh?t tr?ng thi t?n cng (t? t?t)
    updateStatus();
}

void Player::draw(sf::RenderWindow& window) {
    window.draw(playerShape);
}

sf::FloatRect Player::getCollisionBox() const {
    return playerShape.getGlobalBounds();
}

sf::FloatRect Player::getHurtBox() const {
    return playerShape.getGlobalBounds();
}
