#include "Ally.h"
#include "BalanceConfig.h"
#include "GameContext.h"
#include "MathUtils.h"
#include <cmath>
#include <iostream>
#include "TextureManager.h"
#include "Weapon.h"

// ===============================
// CONSTRUCTORS
// ===============================


Ally::Ally(float x, float y, TextureManager& textureManager, const std::string& textureName)
    : Entity(
          x,
          y,
          BalanceConfig::ALLY_MAX_HEALTH,
          BalanceConfig::ALLY_MAX_HEALTH
      ),
      allyTexture(nullptr) {
    team = Team::Player;
    attackCoolDown = 2.f;
    coolDownTimer = 0.f;

    sf::Texture& tex = textureManager.getTexture(textureName);
    allyTexture = &tex;

    const float desiredWidth = 40.f;
    const float desiredHeight = 40.f;
    const sf::Vector2u textureSize = allyTexture->getSize();

    if (textureSize.x > 0 && textureSize.y > 0) {
        const float scaleX = desiredWidth / static_cast<float>(textureSize.x);
        const float scaleY = desiredHeight / static_cast<float>(textureSize.y);
        rectShape.setSize(sf::Vector2f(
            static_cast<float>(textureSize.x),
            static_cast<float>(textureSize.y)));
        rectShape.setTexture(allyTexture);
        rectShape.setScale(sf::Vector2f(scaleX, scaleY));
        rectShape.setOrigin(sf::Vector2f(textureSize.x / 2.f, textureSize.y / 2.f));
    }
    else {
        rectShape.setSize({ desiredWidth, desiredHeight });
        rectShape.setOrigin({ desiredWidth / 2.f, desiredHeight / 2.f });
        rectShape.setFillColor(sf::Color::Cyan);
    }
    rectShape.setPosition(sf::Vector2f(x, y));
}

// ===============================
// CORE LOGIC
// ===============================

void Ally::update(GameContext& context) {
    Entity::update(context);

    if (isDead()) {
        if (!isDying) startDying();
        updateDeadTimer(context);
        rectShape.setPosition(getPosition());
        return;
    }

    // 1. Cập nhật mục tiêu
    updateTarget(context);

    // 2. Nếu có mục tiêu hợp lệ
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

        if (currentWeapon && isInRange(target) && coolDownTimer <= 0.f) {
            startAttacking();
        }
    }

    if (isAttacking) {
        updateAttackTimer(context);
        if (currentWeapon && context.combatManager) {
            currentWeapon->triggerAction(this, context, *context.combatManager);
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

bool Ally::isInRange(const Entity* target) const {
    if (!target) return false;
    float dx = target->getX() - getX();
    float dy = target->getY() - getY();
    return (dx * dx + dy * dy) <= (range * range);
}

//logic tim kiem va chon quai gan nhat lam muc tieu va danh den khi quai do chet 

void Ally::updateTarget(const GameContext& context) {
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
