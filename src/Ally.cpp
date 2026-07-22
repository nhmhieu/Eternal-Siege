#include "Ally.h"
#include "GameContext.h"
#include <cmath>
#include <iostream>
#include "TextureManager.h"

Ally::Ally(float x, float y, TextureManager& textureManager, const std::string& textureName)
    : Entity(x, y, 50, 50) {
    if (!textureManager.loadTexture(textureName, "assets/images/" + textureName + ".png")) {
        std::cerr << "Failed to load " << textureName << " texture!" << std::endl;
    }
    sprite.setTexture(textureManager.getTexture(textureName));
    sprite.setPosition(sf::Vector2f(x, y));
    sprite.setScale({ 0.5f, 0.5f });
    team = Team::Player;
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

    if (target) {
        cooldownTimer += context.deltaTime;
        if (cooldownTimer >= attackCoolDown) {
            cooldownTimer = 0.f;
            target->takeDamage(10.f);
            std::cout << "Ally attacked! Monster health: " << target->getHealth() << std::endl;
        }
    }
    sprite.setPosition(getPosition());
}

void Ally::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}