#pragma once 

#include "Entity.h" 
#include <iostream>
#include <vector>

class Monster : public Entity{ 

    private : 
        float attackRange ; 
        float speed ; 
        Entity* currentTarget ; 
        float targetTimer ; 



    public : 
        ~Monster() ;
        Monster() ;
        Monster(float x, float y, float health, float maxHealth) ;


        //ham update target cua quai vat 
        void updateTarget(const std :: vector<Entity*>& allTargets) ;
        //ham cho quai di chuyen den target hien tai
        void moveToward(float deltaTime, Entity* target) ; 
        void update(float deltaTime, const std :: vector<Entity*>& allTargets) ; 
} ;