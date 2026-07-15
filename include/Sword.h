#pragma once

#include "Weapon.h"
#include <cmath>

class Sword : public Weapon {
private:
    int damage;
    float range;

public:
    Sword(int dmg, float rng) : damage(dmg), range(rng) {}

    sf::FloatRect getHitbox(sf::Vector2f entityCenter, sf::Vector2f attackDir) override {
        constexpr float hitboxSize = 40.f;
        sf::Vector2f pos = entityCenter + (attackDir * range) - sf::Vector2f(hitboxSize / 2.f, hitboxSize / 2.f);
        return sf::FloatRect(pos, sf::Vector2f(hitboxSize, hitboxSize));
    }

    bool isHitting(sf::Vector2f attackerPos, sf::Vector2f attackDir, sf::Vector2f targetPos) override {
        sf::Vector2f dirToTarget = targetPos - attackerPos;
        float distance = std::sqrt(dirToTarget.x * dirToTarget.x + dirToTarget.y * dirToTarget.y);
        if (distance > range) return false;

        sf::Vector2f normalizedDir = dirToTarget / distance;
        float dot = (attackDir.x * normalizedDir.x) + (attackDir.y * normalizedDir.y);

        constexpr float cosHalfAngle = 0.70710678f; // cos(45°) cho góc quạt 90°
        return dot >= cosHalfAngle;
    }

    void setDamage(int dmg) override { damage = dmg; }
    int getDamage() const override { return damage; }
};