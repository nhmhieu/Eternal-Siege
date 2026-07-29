#include "Ally.h"
#include "GameContext.h"
#include <cmath>
#include <iostream>
#include "TextureManager.h"

Ally::Ally(float x, float y) : Entity(x, y, 50, 50) {
    team = Team::Player;
    rectShape.setFillColor(sf::Color::Cyan);
    rectShape.setSize(sf::Vector2f(50.f, 50.f));
    rectShape.setOrigin(sf::Vector2f(25.f, 25.f));
    rectShape.setPosition(sf::Vector2f(x, y));
}

Ally::Ally(float x, float y, const std::string& textureName)
    : Entity(x, y, 50, 50), allyTexture(nullptr) {
    team = Team::Player;

    sf::Texture& tex = TextureManager::getInstance().getTexture(textureName);
    allyTexture = &tex;

    const float desiredWidth = 40.f;
    const float desiredHeight = 40.f;
    const sf::Vector2u textureSize = allyTexture->getSize();

    float scaleX = desiredWidth / static_cast<float>(textureSize.x);
    float scaleY = desiredHeight / static_cast<float>(textureSize.y);

    rectShape.setSize(sf::Vector2f(static_cast<float>(textureSize.x), static_cast<float>(textureSize.y)));
    rectShape.setTexture(allyTexture);
    rectShape.setScale(sf::Vector2f(scaleX, scaleY));
    rectShape.setOrigin(sf::Vector2f(textureSize.x / 2.f, textureSize.y / 2.f));
    rectShape.setPosition(sf::Vector2f(x, y));
}

void Ally::update(const GameContext& context) {
    Entity* target = nullptr;
    float minDist = attackRange + 1.f;

    for (auto* entity : context.allEntity) {
        if (entity->getTeam() == Team::Enemy && !entity->isDead()) {
            float dx = entity->getX() - getX();
            float dy = entity->getY() - getY();
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist < minDist) {
                minDist = dist;
                target = entity;
            }
        }
    }

    if (target && minDist <= attackRange) {
        cooldownTimer += context.deltaTime;
        if (cooldownTimer >= attackCoolDown) {
            cooldownTimer = 0.f;
            target->takeDamage(10.f);
        }
    }

    rectShape.setPosition(getPosition());
}

void Ally::draw(sf::RenderWindow& window) {
    window.draw(rectShape);
}

sf::FloatRect Ally::getCollisionBox() const {
    return rectShape.getGlobalBounds();
}

sf::FloatRect Ally::getHurtBox() const {
    return rectShape.getGlobalBounds();
}