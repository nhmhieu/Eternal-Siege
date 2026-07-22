#include "Player.h"
#include "GameContext.h"
#include <cmath>
#include <iostream>
#include "TextureManager.h"


Player::Player(TextureManager& textureManager)
    : Entity(400.f, 300.f, 100, 100), playerTexture(nullptr) {
    team = Team::Player;

    const float desiredSize = 80.f;

    if (!textureManager.getTexture("Ash").getSize().x) {
        std::cerr << "Failed to load Ash texture!" << std::endl;
        useFallback = true;
        fallbackShape.setFillColor(sf::Color::Blue);
        fallbackShape.setSize(sf::Vector2f(desiredSize, desiredSize));
        fallbackShape.setOrigin(sf::Vector2f(desiredSize / 2.f, desiredSize / 2.f));
        fallbackShape.setPosition(sf::Vector2f(400.f, 300.f));
    } else {
        std::cout << "Ash texture loaded OK!" << std::endl;
        useFallback = false;
        playerTexture = &textureManager.getTexture("Ash");
        playerShape.setSize(sf::Vector2f(desiredSize, desiredSize));
        playerShape.setOrigin(sf::Vector2f(desiredSize / 2.f, desiredSize / 2.f));
        playerShape.setTexture(playerTexture);
        playerShape.setPosition(sf::Vector2f(400.f, 300.f));
    }
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

    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !isAttacking) {
        setIsAttacking(true);
    }
}

void Player::update(const GameContext& context) {
    sf::Vector2f dir = getDirection();
    if (dir.x != 0.f || dir.y != 0.f) {
        float newX = getX() + dir.x * speed * context.deltaTime;
        float newY = getY() + dir.y * speed * context.deltaTime;
        setPosition(newX, newY);
        playerShape.setPosition(getPosition());
        fallbackShape.setPosition(getPosition());
    }
}

void Player::draw(sf::RenderWindow& window) {
    if (useFallback) {
        window.draw(fallbackShape);
    } else {
        window.draw(playerShape);
    }
}

sf::FloatRect Player::getCollisionBox() const {
    if (useFallback) return fallbackShape.getGlobalBounds();
    return playerShape.getGlobalBounds();
}

sf::FloatRect Player::getHurtBox() const {
    if (useFallback) return fallbackShape.getGlobalBounds();
    return playerShape.getGlobalBounds();
}
