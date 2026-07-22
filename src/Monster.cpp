#include "Monster.h"
#include "GameContext.h"
#include <cmath>
#include <iostream>
#include <limits>

Monster::Monster(float x, float y, float health, float maxHealth,
    float range, float cooldown, float spd, float dmg)
    : Entity(x, y, health, maxHealth),
    attackRange(range),
    attackCooldown(cooldown),
    attackDamage(dmg),
    speed(spd) {
    sprite.setRadius(20.f);
    sprite.setFillColor(sf::Color::Red);
    sprite.setOrigin(sf::Vector2f(20.f, 20.f));
    sprite.setPosition(sf::Vector2f(x, y));
    team = Team::Enemy;
}

void Monster::updateTarget(const std::vector<Entity*>& targets) {
    if (targets.empty()) {
        std::cout << "Monster: targets is empty" << std::endl;
        currentTarget = nullptr;
        return;
    }
    
    
    Entity* closest = nullptr;
    float minDistSq = std::numeric_limits<float>::max();

    for (auto* target : targets) {
        if (!target || target->isDead()) continue;
        float dx = target->getX() - getX();
        float dy = target->getY() - getY();
        float distSq = dx * dx + dy * dy;
        if (distSq < minDistSq) {
            minDistSq = distSq;
            closest = target;
        }
    }
    currentTarget = closest;

    sf :: Vector2f attackDir = currentTarget->getPosition() - this->getPosition() ; 
    this->setAttackDirection(attackDir) ;  
    

}

void Monster::moveToward(float deltaTime) {
    if (!currentTarget) {
        std::cout << "Monster: no target!" << std::endl;
        return;
    }

    float dx = currentTarget->getX() - getX();
    float dy = currentTarget->getY() - getY();
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance <= 0.001f) return;

    if (distance <= attackRange) {
        attackTimer += deltaTime;
        this->setIsAttacking(true) ; 
        if (attackTimer >= attackCooldown) {
            attackTimer = 0.f;
            this->setIsAttacking(false) ;
            // currentTarget->takeDamage(attackDamage);
            // std::cout << "Monster attacked! Target health: " << currentTarget->getHealth() << std::endl;
        }
        return;
    }

    float moveX = (dx / distance) * speed * deltaTime;
    float moveY = (dy / distance) * speed * deltaTime;
    setX(getX() + moveX);
    setY(getY() + moveY);
    sprite.setPosition(position);
}

void Monster::update(const GameContext& context) {
    // N?u m?c ti�u ch?t ho?c kh�ng c�, t�m m?c ti�u m?i ngay
    if (currentTarget == nullptr || currentTarget->isDead()) {
        updateTarget(context.players);
    }

    targetTimer += context.deltaTime;
    if (targetTimer >= 0.5f) {
        targetTimer = 0.f;
        updateTarget(context.players);
    }

    if (currentTarget && !currentTarget->isDead()) {
        moveToward(context.deltaTime);
    }
    else {
        currentTarget = nullptr;
    }

    if(isDying){  //neu nhu flag dang chet duoc bat thi bat dau dem timer cho animation chet
        updateDeadTimer(context) ;
    }
    if(isAttacking){
        attackClock.restart() ; 
    }
    updateStatus();
}

void Monster::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}