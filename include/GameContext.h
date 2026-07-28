#pragma once

#include <vector>
#include "CombatManager.h"
#include "Projectiles.h" 
#include "Entity.h" 
#include "Ally.h" 
#include "Monster.h"  

class Projectiles;
class Entity;
class GameContext {
public:
    float deltaTime = 0.f;
    std::vector<Entity*> allEntity;

    // Các danh sách chuyên biệt
    std::vector<Entity*> players;
    std::vector<Entity*> enemies;
    std::vector<Monster*> monsters;
    std::vector<Ally*> allies;
    std::vector<Projectiles*> projectiles;

    CombatManager* combatManager = nullptr;

    GameContext() = default;
    ~GameContext() = default;

    // Ngăn copy để tránh double-free
    GameContext(const GameContext&) = delete;
    GameContext& operator=(const GameContext&) = delete;

    GameContext(float dt, const std::vector<Entity*>& entities)
        : deltaTime(dt), allEntity(entities) {
    }
};