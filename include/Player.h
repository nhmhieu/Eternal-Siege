#pragma once

#include "Entity.h"
#include <SFML/Graphics.hpp>
#include <iostream>

class GameContext;

class Player : public Entity {
private:
    float speed = 300.f;

public:
    Player();
    ~Player() override = default;

    void handleInput();
    void update(const GameContext& context) override;
    void draw(sf::RenderWindow& window) override;
    bool canAttack(){
        // std :: cout << "cooLDownTimer : " << coolDownTimer << std :: endl ;
        bool isReady = coolDownTimer <= 0 ; 
        return isReady ;
    }

    void updatePlayerStatus() {
    if (isAttacking) {
        if (attackTimer >= attackDuration) {
            isAttacking = false;
            if (currentWeapon) currentWeapon->clearHitList();
            coolDownTimer = attackCoolDown ; 
            attackTimer = 0.f ; 

        }
    }
}
};