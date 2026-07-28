#include "Ally.h"
#include "GameContext.h"
#include "MathUtils.h"
#include <cmath>
#include <iostream>
#include "TextureManager.h"

// ===============================
// CONSTRUCTORS
// ===============================


Ally::Ally(float x, float y, TextureManager& textureManager, const std::string& textureName)
    : Entity(x, y, 50, 50), allyTexture(nullptr) {
    team = Team::Player;

    sf::Texture& tex = textureManager.getTexture(textureName);
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

// ===============================
// CORE LOGIC
// ===============================

void Ally::update(GameContext& context) {
    // 1. Giảm cooldown
    if (coolDownTimer > 0.f) {
        coolDownTimer -= context.deltaTime;
    }

    // 2. Cập nhật mục tiêu
    updateTarget(context);

    // 3. Nếu có mục tiêu hợp lệ
    if (target && !target->isDead()) {
        sf::Vector2f dir = target->getPosition() - this->getPosition();
        float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (length != 0.f) {
            dir /= length;
        }
        else {
            dir = { 1.f, 0.f };
        }
        this->setAttackDirection(dir);

        if (isInRange(target) && coolDownTimer <= 0.f) {
            startAttacking();
        }
    }

    if (isDying) {
        updateDeadTimer(context);
    }

    if (isAttacking) {
        updateAttackTimer(context);
        if (currentWeapon) {
            currentWeapon->triggerAction(this, context, *context.combatManager);
        }
        else {
            if (target && !target->isDead() && isInRange(target)) {
                target->takeDamage(10.f);
                std::cout << "Ally attacked (fallback)! Target health: " << target->getHealth() << std::endl;
            }
        }
    }

    updateStatus();

    rectShape.setPosition(getPosition());
}

void Ally::draw(sf::RenderWindow& window) {
    window.draw(rectShape);
}

// ===============================
// TARGET MANAGEMENT
// ===============================

bool Ally::isInRange(Entity* target) {
    if (!target) return false;
    float dx = target->getX() - getX();
    float dy = target->getY() - getY();
    return (dx * dx + dy * dy) <= (range * range);
}

//logic tim kiem va chon quai gan nhat lam muc tieu va danh den khi quai do chet 

void Ally::updateTarget(const GameContext& context) {
    if (target != nullptr && target->isDead()) {
        target = nullptr;
    }

    Entity* bestTarget = nullptr;
    float minDistSq = range * range;

    for (auto* entity : context.enemies) {
        if (entity == nullptr || entity->isDead()) continue;
        float dx = entity->getX() - getX();
        float dy = entity->getY() - getY();
        float distSq = dx * dx + dy * dy;
        if (distSq <= minDistSq) {
            minDistSq = distSq;
            bestTarget = entity;
        }
    }

    target = bestTarget;
}
sf::FloatRect Ally::getCollisionBox() const {
    return rectShape.getGlobalBounds();
}

sf::FloatRect Ally::getHurtBox() const {
    return rectShape.getGlobalBounds();
}
