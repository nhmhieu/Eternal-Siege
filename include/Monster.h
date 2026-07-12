#pragma once 

#include "Entity.h" 
#include <iostream>
#include <vector>
#include "GameContext.h"

class Monster : public Entity{ 

    private : 
    
        float attackRange ; 
        float speed ; 
        Entity* currentTarget ; 
        float targetTimer ; 

        //bien sprite de ve quai vat
        sf :: CircleShape sprite ; 



    public : 
        ~Monster() ;
        Monster() ;
        Monster(float x, float y, float health, float maxHealth) ;


        //ham update target cua quai vat 
        void updateTarget(const std :: vector<Entity*>& allTargets) ;
        //ham cho quai di chuyen den target hien tai
        void moveToward(float deltaTime, Entity* target) ; 
        void update(const GameContext& context) override ; 

        ///Ham ve quai vat ra man hinh
        void draw(sf :: RenderWindow& window) override ; 
} ;