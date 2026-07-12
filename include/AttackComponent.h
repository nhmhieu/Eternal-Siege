#pragma once 

#include <iostream>
#include <vector>

class Entity ; 

class AttackComponent{

    private :
        float cooldown ;
        float range ; 
        float damage ; 

    public : 
        bool canAttack(float dt) ;
        void performAttack(Entity& attacker, std :: vector<Entity*>& target) ;     

} ;