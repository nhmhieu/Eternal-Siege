#pragma once

#include <vector>
// #include "Projectiles.h" 
// #include "Entity.h" 
 
class Projectiles ; 
class Entity ; 
class GameContext {
public:
    float deltaTime = 0.f;
    std::vector<Entity*> allEntity;
    std::vector<Entity*> players;
    std::vector<Entity*> enemies;
    std :: vector<Projectiles*> projectiles ; 

    GameContext() = default;
    ~GameContext(){
        // for(auto* it : allEntity) delete it ; 

        // for(auto* it : projectiles) delete it ; 
    }
    GameContext(float dt, const std::vector<Entity*>& entities)
        : deltaTime(dt), allEntity(entities) {
    }
};