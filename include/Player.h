#pragma once

#include "Entity.h"
#include <SFML/Graphics.hpp>
#include "TextureManager.h"
#include <iostream>
#include "Weapon.h"

class GameContext;

class Player : public Entity {
private:
    float speed = 300.f;
    const sf::Texture* playerTexture;
    sf::RectangleShape playerShape;


public:
    Player(TextureManager& textureManager);

    void handleInput();
    void update(GameContext& context) override;
    void draw(sf::RenderWindow& window) override;

    bool canAttack() {
        return coolDownTimer <= 0;
    }

    void updateStatus()  {
        if (isAttacking) {
            if (attackTimer >= attackDuration) {
                isAttacking = false;
                if (currentWeapon) {
                    currentWeapon->clearHitList();
                }
                coolDownTimer = attackCoolDown;
                attackTimer = 0.f;
            }
        }
    }
    sf::FloatRect getCollisionBox() const override;
    sf::FloatRect getHurtBox() const override;
};