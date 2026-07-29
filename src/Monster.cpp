#include "Monster.h"
#include "GameContext.h"
#include <cmath>
#include <iostream>
#include <limits>

// ===============================
// CONSTRUCTORS
// ===============================

Monster::Monster() {
    team = Team::Enemy;
}

Monster::Monster(float x, float y, float health, float maxHealth,
    float range, float cooldown, float spd, float dmg)
    : Entity(x, y, health, maxHealth),
    attackRange(range),
    attackCooldown(cooldown),
    attackDamage(dmg),
    speed(spd) {

    // Khởi tạo shape màu đỏ (fallback)
    monsterShape.setFillColor(sf::Color::Red);
    monsterShape.setSize(sf::Vector2f(30.f, 30.f));
    monsterShape.setOrigin(sf::Vector2f(15.f, 15.f));
    monsterShape.setPosition(sf::Vector2f(x, y));

    setPosition(x, y);
    team = Team::Enemy;
    isAlive = true;
    attackPower = dmg;
    attackCoolDown = cooldown;
    coolDownTimer = cooldown;
}

// ===============================
// TARGET MANAGEMENT
// ===============================

void Monster::updateTarget(const std::vector<Entity*>& targets) {
    if (targets.empty()) {
        // std::cout << "Monster: targets is empty" << std::endl;
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

    // Cập nhật hướng tấn công nếu có mục tiêu
    if (currentTarget) {
        sf::Vector2f attackDir = currentTarget->getPosition() - this->getPosition();
        float length = std::sqrt(attackDir.x * attackDir.x + attackDir.y * attackDir.y);
        if (length != 0.f) {
            attackDir /= length;
        }
        this->setAttackDirection(attackDir);
    }
}

void Monster::moveToward(float deltaTime) {
    if (!currentTarget) {
        // std::cout << "Monster: no target!" << std::endl;
        return;
    }

    float dx = currentTarget->getX() - getX();
    float dy = currentTarget->getY() - getY();
    float distance = std::sqrt(dx * dx + dy * dy);

    // Cập nhật gap (khoảng cách đến mục tiêu)
    gap = distance;

    if (distance <= 1.f) return;

    // Trong tầm đánh thì dừng di chuyển; update() sẽ kiểm tra cooldown
    // trước khi bật trạng thái tấn công.
    if (distance <= attackRange) {
        return;
    }

    // Di chuyển về phía mục tiêu
    float moveX = (dx / distance) * speed * deltaTime;
    float moveY = (dy / distance) * speed * deltaTime;
    setX(getX() + moveX);
    setY(getY() + moveY);
    monsterShape.setPosition(getPosition());
}

// ===============================
// UPDATE & DRAW
// ===============================

void Monster::update(GameContext& context) {
    // 1. Giảm cooldown
    if (coolDownTimer > 0.f) {
        coolDownTimer -= context.deltaTime;
    }

    // 2. Nếu chết, cập nhật timer chết và thoát
    if (isDying) {
        updateDeadTimer(context);
        monsterShape.setPosition(getPosition());
        return;
    }

    // Re-evaluate before dereferencing the previous raw target. A target may
    // have been removed during the previous frame's cleanup.
    updateTarget(context.players);
    targetTimer += context.deltaTime;
    if (targetTimer >= 0.5f) {
        targetTimer = 0.f;
    }

    // 4. Di chuyển về phía mục tiêu
    if (currentTarget && !currentTarget->isDead()) {
        moveToward(context.deltaTime);
    }
    else {
        currentTarget = nullptr;
    }

    // 5. Xử lý tấn công
    if (currentTarget && !currentTarget->isDead() && canAttack()) {
        startAttacking();
    }

    if (isAttacking) {
        updateAttackTimer(context);
        if (getCurrentWeapon() != nullptr) {
            getCurrentWeapon()->triggerAction(this, context, *(context.combatManager));
        }
        else {
            // Fallback: gây sát thương trực tiếp
            if (currentTarget && !currentTarget->isDead() && gap <= attackRange) {
                currentTarget->takeDamage(attackDamage);
                std::cout << "Monster attacked (fallback)! Target health: " << currentTarget->getHealth() << std::endl;
            }
        }
    }

    // 6. Cập nhật trạng thái (tự tắt isAttacking nếu hết thời gian)
    updateStatus();

    // 7. Đồng bộ vị trí shape
    monsterShape.setPosition(getPosition());
}

void Monster::draw(sf::RenderWindow& window) {
    // Vẽ shape (có thể thay bằng sprite sau)
    window.draw(monsterShape);
}

sf::FloatRect Monster::getCollisionBox() const {
    return monsterShape.getGlobalBounds();
}

sf::FloatRect Monster::getHurtBox() const {
    return monsterShape.getGlobalBounds();
}
