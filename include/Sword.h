#pragma once

#include "Weapon.h"
#include <cmath>

class CombatManager ;
class GameContext ; 
class Sword : public Weapon {
private:
    int damage;
    float range;

public:
    Sword(int dmg, float rng) : damage(dmg), range(rng) {}


    sf::FloatRect getHitbox(sf::Vector2f entityCenter, sf::Vector2f attackDir) override ;

    bool isHitting(sf::Vector2f attackerPos, sf::Vector2f attackDir, sf::Vector2f targetPos) override ;
    void setDamage(int dmg) override { damage = dmg; }
    int getDamage() const override { return damage; }

    void triggerAction(Entity* attacker,GameContext& context, CombatManager& combatManager) override ;
};