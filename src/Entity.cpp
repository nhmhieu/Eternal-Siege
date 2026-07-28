#include "Entity.h"
#include "Weapon.h"
#include <iostream>
#include "GameContext.h"
#include <cmath>   // nếu dùng sqrt

Entity::Entity()
    : position(0, 0), health(0), maxHealth(0), sprite(defaultTexture) {
}

Entity::Entity(float x, float y, float health, float maxHealth)
    : position(x, y), health(health), maxHealth(maxHealth), sprite(defaultTexture) {
}

Entity::Entity(float x, float y, float health, float maxHealth, Team team, std::unique_ptr<Weapon> weapon)
    : position(x, y), health(health), maxHealth(maxHealth), team(team), currentWeapon(std::move(weapon)), sprite(defaultTexture) {
}

void Entity::takeDamage(float damage) {
    health -= damage;
    if (health <= 0) {
        health = 0;
        isAlive = false;
        std::cout << "Entity died!" << std::endl;
    }
}

sf::FloatRect Entity::getCollisionBox() const {
    return sprite.getGlobalBounds();
}

sf::FloatRect Entity::getHurtBox() const {
    return sprite.getGlobalBounds();
}

sf::FloatRect Entity::getAttackHitbox() const {
    sf::FloatRect bounds = sprite.getGlobalBounds();
    sf::Vector2f center = bounds.position + (bounds.size / 2.f);
    float range = 40.f;
    sf::Vector2f hitboxSize(40.f, 40.f);
    sf::Vector2f attackCenter = center + (attackDirection * range);
    sf::Vector2f attackPos = attackCenter - (hitboxSize / 2.f);
    return sf::FloatRect(attackPos, hitboxSize);
}

void Entity::updateStatus() {
    if (isAttacking) {
        if (attackTimer >= attackDuration) {
            isAttacking = false;
            if (currentWeapon) {
                currentWeapon->clearHitList();
                // Chỉ gọi nếu có setHasAttacked trong Weapon
                // currentWeapon->setHasAttacked(false);
            }
            coolDownTimer = attackCoolDown;
            attackTimer = 0.f;
        }
    }
}

void Entity::drawHealthBar(sf::RenderWindow& window) const {
    const float barWidth = 50.f;
    const float barHeight = 6.f;
    const float offsetY = -40.f;

    sf::Vector2f pos = position;
    sf::Vector2f barPos(pos.x - barWidth / 2.f, pos.y + offsetY);

    sf::RectangleShape background(sf::Vector2f(barWidth, barHeight));
    background.setFillColor(sf::Color(60, 60, 60));
    background.setPosition(barPos);
    window.draw(background);

    float healthPercent = (maxHealth > 0.f) ? (health / maxHealth) : 0.f;
    if (healthPercent < 0.f) healthPercent = 0.f;

    sf::RectangleShape healthBar(sf::Vector2f(barWidth * healthPercent, barHeight));
    if (team == Team::Enemy) {
        healthBar.setFillColor(sf::Color::Red);
    }
    else {
        healthBar.setFillColor(sf::Color::Green);
    }
    healthBar.setPosition(barPos);
    window.draw(healthBar);
}

void Entity::updateDeadTimer(const GameContext& context) {
    if (isDying) {
        deadTimer += context.deltaTime;
        // logic animation chết
    }
}

void Entity::updateAttackTimer(const GameContext& context) {
    if (isAttacking) {
        attackTimer += context.deltaTime;
    }
}

void Entity::update(GameContext& context) {
    // Giảm cooldown timer
    if (coolDownTimer > 0) {
        coolDownTimer -= context.deltaTime;
        if (coolDownTimer < 0) coolDownTimer = 0;
    }
}