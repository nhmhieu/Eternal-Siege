#pragma once

#include <vector>
#include <unordered_set>
#include "Entity.h"
#include "Weapon.h"

class CombatManager {
private:
    std::unordered_set<Entity*> hitEntities;

public:
    void processAttack(Entity* attacker, Weapon* weapon, std::vector<Entity*>& targets);
    void resetAttack();
    bool hasHit(Entity* target) const;
    void processProjectiles(const GameContext& context, const std :: vector<Entity*>& target) ; 
};