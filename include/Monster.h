#pragma once

#include "Entity.h"
#include <vector>
#include <iostream>

class GameContext;

class Monster : public Entity {
private:
    float attackRange = 50.f;
    float attackCooldown = 1.f;
    float attackDamage = 10.f;
    float speed = 100.f;
    Entity* currentTarget = nullptr;
    float targetTimer = 0.f;
    float attackTimer = 0.f;


public:
    Monster() ;
    Monster(float x, float y, float health = 100, float maxHealth = 100,
        float range = 50.f, float cooldown = 1.f, float spd = 100.f, float dmg = 10.f);
    ~Monster() override = default;

    void updateTarget(const std::vector<Entity*>& targets);
    void moveToward(float deltaTime);

    void update(GameContext& context) override;
    void draw(sf::RenderWindow& window) override;

    Entity* getCurrentTarget() const { return currentTarget; }

    bool canAttack(){
        // std :: cout << gap << std :: endl ;
        bool isInRange = (attackRange >= gap) ; 
        bool isReady = !isAttacking && coolDownTimer <= 0 ;

        // if(!isInRange) std :: cout << "Khong trong tam danh" << std :: endl ; 
        // if(!isReady) std :: cout << "Quai chua san sang tan cong " << std :: endl ; 
        
        return isInRange && isReady ; 
    }
};