#include "Entity.h"
#include <iostream>

Entity::Entity()
    : position(0, 0), health(0), maxHealth(0), sprite(defaultTexture) {
}

Entity::Entity(float x, float y, float health, float maxHealth)
    : position(x, y), health(health), maxHealth(maxHealth), sprite(defaultTexture) {
}

Entity::Entity(float x, float y, float health, float maxHealth, Team team, Weapon* weapon)
    : position(x, y), health(health), maxHealth(maxHealth), team(team), currentWeapon(weapon), sprite(defaultTexture) {
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
        if (attackClock.getElapsedTime().asSeconds() > attackDuration) {
            isAttacking = false;
            if (currentWeapon) currentWeapon->clearHitList();
        }
    }
}

void Entity::drawHealthBar(sf::RenderWindow& window) const {
    const float barWidth = 50.f;
    const float barHeight = 6.f;
    const float offsetY = -40.f; // Phía trên entity

    sf::Vector2f pos = position;
    sf::Vector2f barPos(pos.x - barWidth / 2.f, pos.y + offsetY);

    // Nền (màu xám đen)
    sf::RectangleShape background(sf::Vector2f(barWidth, barHeight));
    background.setFillColor(sf::Color(60, 60, 60));
    background.setPosition(barPos);
    window.draw(background);

    // Thanh máu
    float healthPercent = (maxHealth > 0.f) ? (health / maxHealth) : 0.f;
    if (healthPercent < 0.f) healthPercent = 0.f;

    sf::RectangleShape healthBar(sf::Vector2f(barWidth * healthPercent, barHeight));
    // Màu sắc dựa trên team
    if (team == Team::Enemy) {
        healthBar.setFillColor(sf::Color::Red);
    } else {
        healthBar.setFillColor(sf::Color::Green);
    }
    healthBar.setPosition(barPos);
    window.draw(healthBar);
}
