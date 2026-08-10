#pragma once

#include "Weapon.h"
#include <cmath>

class CombatManager ;
class GameContext ; 
class Sword : public Weapon {
private:
    float range;

public:
    explicit Sword(float rng) : range(rng) {}


    sf::FloatRect getHitbox(sf::Vector2f entityCenter, sf::Vector2f attackDir) override ;

    bool isHitting(sf::Vector2f attackerPos, sf::Vector2f attackDir, sf::Vector2f targetPos) override ;
    float getAttackRange() const override { return range; }

    void triggerAction(Entity* attacker,GameContext& context, CombatManager& combatManager) override ;
};
