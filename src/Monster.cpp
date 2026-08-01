#include "Monster.h"
#include "GameContext.h"
#include "TextureManager.h"
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
    // Monster luôn dùng shape màu đỏ
    //monsterShape.setFillColor(sf::Color::Red);
    //monsterShape.setSize(sf::Vector2f(30.f, 30.f));
    //monsterShape.setOrigin(sf::Vector2f(15.f, 15.f));
    //monsterShape.setPosition(sf::Vector2f(x, y));

    setPosition(x, y);
    team = Team::Enemy;

    // Nạp texture cho monster
    auto& monsterTexture = TextureManager::getInstance().getTexture("monster");
    monsterSprite = std::make_unique<sf::Sprite>(monsterTexture);

    // Đặt vị trí ban đầu
    monsterSprite->setPosition(sf::Vector2f(static_cast<float>(x), static_cast<float>(y)));

    // Lấy kích thước gốc của texture
    sf::Vector2u texSize = monsterTexture.getSize();

    // Giả sử ENtiny hiển thị ở kích thước 64x64
    float desiredWidth = 75.f;
    float desiredHeight = 60.f;

    // Tính hệ số scale
    float scaleX = desiredWidth / static_cast<float>(texSize.x);
    float scaleY = desiredHeight / static_cast<float>(texSize.y);

    // Áp dụng scale (chú ý cú pháp Vector2f)
    monsterSprite->setScale(sf::Vector2f(scaleX, scaleY));

    monsterSprite->setPosition(sf::Vector2f(x, y));
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
        if (attackTimer >= attackCooldown) {
            attackTimer = 0.f;
            currentTarget->takeDamage(attackDamage);
            std::cout << "Monster attacked! Target health: " << currentTarget->getHealth() << std::endl;
        }
        return;
    }

    float moveX = (dx / distance) * speed * deltaTime;
    float moveY = (dy / distance) * speed * deltaTime;
    setX(getX() + moveX);
    setY(getY() + moveY);
    if (monsterSprite) monsterSprite->setPosition(position);
}

void Monster::update(const GameContext& context) {
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

    updateStatus();
}

void Monster::draw(sf::RenderWindow& window) {
    if (monsterSprite) 
        window.draw(*monsterSprite);
}

sf::FloatRect Monster::getCollisionBox() const {
    return monsterSprite ? monsterSprite->getGlobalBounds() : sf::FloatRect();
}

sf::FloatRect Monster::getHurtBox() const {
    return monsterSprite ? monsterSprite->getGlobalBounds() : sf::FloatRect();
}