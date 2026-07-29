#include "Bow.h"
#include "Arrow.h"
#include "CombatManager.h"
#include <iostream>

Bow::Bow() {}

Bow::Bow(float dmg, float cooldown) {
    this->damage = static_cast<int>(dmg);
    this->cooldown = cooldown;
}

Bow::~Bow() {}

sf::FloatRect Bow::getHitbox(sf::Vector2f entityCenter, sf::Vector2f) {
    // Tạm thời: hitbox nhỏ xung quanh entity
    return sf::FloatRect(entityCenter - sf::Vector2f(15.f, 15.f), sf::Vector2f(30.f, 30.f));
}

bool Bow::isHitting(sf::Vector2f attackerPos, sf::Vector2f, sf::Vector2f targetPos) {
    // Kiểm tra khoảng cách đơn giản
    float dx = targetPos.x - attackerPos.x;
    float dy = targetPos.y - attackerPos.y;
    return (dx * dx + dy * dy) <= 250.f * 250.f;
}

void Bow::fire(sf::Vector2f position, sf::Vector2f direction, Team shooterTeam, GameContext& context) {
    Arrow* newArrow = new Arrow(position, direction, projectTileSpeed, damage, shooterTeam);
    context.projectiles.push_back(newArrow);
    std::cout << "Da ban !!!" << std::endl;
}

void Bow::setDamage(int dmg) {
    damage = dmg;
}

int Bow::getDamage() const {
    return damage;
}

void Bow::triggerAction(Entity* attacker, GameContext& context, CombatManager&) {
    if (attacker->getIsAttacking() && !hasAttacked) {
        this->fire(attacker->getPosition(), attacker->getAttackDirection(),
            attacker->getTeam(), context);
        hasAttacked = true;
    }
}
