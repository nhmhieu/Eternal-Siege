#pragma once

#include "Weapon.h"

class Wand final : public Weapon {
public:
    explicit Wand(float range = 220.f) : attackRange(range) {}

    sf::FloatRect getHitbox(
        sf::Vector2f entityCenter, sf::Vector2f attackDirection) override;
    bool isHitting(
        sf::Vector2f attackerPosition,
        sf::Vector2f attackDirection,
        sf::Vector2f targetPosition) override;
    float getAttackRange() const override { return attackRange; }
    void triggerAction(
        Entity* attacker,
        GameContext& context,
        CombatManager& combatManager) override;

private:
    float attackRange;
    float projectileSpeed = 440.f;
};
