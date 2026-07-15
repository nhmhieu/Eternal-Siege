#include "Ally.h"
#include "GameContext.h"
#include <cmath>
#include <iostream>

Ally::Ally(float x, float y)
    : Entity(x, y, 50.f, 50.f), attackRange(100.f), attackCoolDown(1.f), cooldownTimer(0.f)
{
    sprite.setRadius(20.f);
    sprite.setFillColor(sf::Color::Blue);
    sprite.setOrigin(sf::Vector2f(20.f, 20.f));
    sprite.setPosition(sf::Vector2f(x, y));
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