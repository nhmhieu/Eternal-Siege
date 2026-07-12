#pragma once 

#include <iostream>
#include <vector>
#include "Entity.h"


class GameContext{

    public : 
        
        float deltaTime ;
        std :: vector<Entity*> allEntity ; 

        GameContext() : deltaTime(0.0f) {} 
        GameContext(float dt, const std :: vector<Entity*>& entities) : deltaTime(dt), allEntity(entities) {}

} ;
