#pragma once

#include <vector>
#include "Entity.h"

class GameContext {
public:
    float deltaTime = 0.f;
    std::vector<Entity*> allEntity;
    std::vector<Entity*> players;
    std::vector<Entity*> enemies;

    GameContext() = default;
    GameContext(float dt, const std::vector<Entity*>& entities)
        : deltaTime(dt), allEntity(entities) {
    }
};