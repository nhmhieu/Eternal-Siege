#pragma once

#include "Entity.h"
#include <SFML/Graphics.hpp>
#include "TextureManager.h"
#include <iostream>
#include "Weapon.h"

class GameContext;
class Map;

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
    void moveWithCollision(sf::Vector2f displacement, const Map& map);

    bool canAttack() {
        return !isAttacking && coolDownTimer <= 0.f;
    }
    void setPosition(const sf::Vector2f& pos);

    sf::FloatRect getCollisionBox() const override;
    sf::FloatRect getHurtBox() const override;
};
