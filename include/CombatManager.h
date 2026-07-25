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
    //Ham nay duyet N X M danh sach projectiles va allEntity. Nen neu nhu co Entity moi thi phai 
    //push vao allEntity trong GameContext
    void processProjectiles(GameContext& context, const std :: vector<Entity*>& target) ; 
};