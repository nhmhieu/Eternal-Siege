#pragma once

#include <vector>
#include "Entity.h"
#include "Weapon.h"

class CombatManager {
public:
    void processAttack(Entity* attacker, Weapon* weapon, std::vector<Entity*>& targets);
};