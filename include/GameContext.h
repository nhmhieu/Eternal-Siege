#pragma once 

#include <iostream>
#include <vector>
#include "Entity.h"


class GameContext{

    public : 
        
        float deltaTime ;
        std::vector<Entity*> allEntity; // Để render và quản lý chung
        std::vector<Entity*> players;     // Danh sách người chơi
        std::vector<Entity*> enemies;     // Danh sách quái vật

        GameContext() : deltaTime(0.0f) {} 
        GameContext(float dt, const std :: vector<Entity*>& entities) : deltaTime(dt), allEntity(entities) {}

} ;
