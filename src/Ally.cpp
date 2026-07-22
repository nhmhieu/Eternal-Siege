#include "Ally.h"
#include "GameContext.h"
#include <cmath>
#include <iostream>
#include "TextureManager.h"

Ally::Ally(float x, float y) : Entity(x, y, 50, 50) {
    team = Team::Player;
    useFallback = true;
    fallbackShape.setFillColor(sf::Color::Cyan);
    fallbackShape.setSize(sf::Vector2f(50.f, 50.f));
    fallbackShape.setOrigin(sf::Vector2f(25.f, 25.f));
    fallbackShape.setPosition(sf::Vector2f(x, y));
}

Ally::Ally(float x, float y, TextureManager& textureManager, const std::string& textureName)
    : Entity(x, y, 50, 50), allyTexture(nullptr) {
    team = Team::Player;

    sf::Texture& tex = textureManager.getTexture(textureName);
    if (!tex.getSize().x) {
        std::cerr << "Texture '" << textureName << "' is empty! Ally uses fallback." << std::endl;
        useFallback = true;
        fallbackShape.setFillColor(sf::Color::Magenta);
        fallbackShape.setSize(sf::Vector2f(50.f, 50.f));
        fallbackShape.setOrigin(sf::Vector2f(25.f, 25.f));
        fallbackShape.setPosition(sf::Vector2f(x, y));
    } else {
        std::cout << "Texture '" << textureName << "' size=" << tex.getSize().x << "x" << tex.getSize().y << std::endl;
        useFallback = false;
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

    debugRect.setSize(sf::Vector2f(50.f, 50.f));
    debugRect.setOrigin(sf::Vector2f(25.f, 25.f));
    debugRect.setPosition(sf::Vector2f(x, y));
    debugRect.setFillColor(sf::Color::Transparent);
    debugRect.setOutlineColor(sf::Color::Red);
    debugRect.setOutlineThickness(2.f);
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

    if (useFallback) {
        fallbackShape.setPosition(getPosition());
    } else {
        rectShape.setPosition(getPosition());
    }
    debugRect.setPosition(getPosition());
}

void Ally::draw(sf::RenderWindow& window) {
    if (useFallback) {
        window.draw(fallbackShape);
    } else {
        window.draw(rectShape);
    }
    window.draw(debugRect);
}

sf::FloatRect Ally::getCollisionBox() const {
    if (useFallback) return fallbackShape.getGlobalBounds();
    return rectShape.getGlobalBounds();
}

sf::FloatRect Ally::getHurtBox() const {
    if (useFallback) return fallbackShape.getGlobalBounds();
    return rectShape.getGlobalBounds();
}
