#pragma once

#include <SFML/Graphics.hpp>
#include "Entity.h"

class GameContext ; 

class Player : public Entity{
private:

    // sf::CircleShape sprite;
    float speed;

public:
    Player();
    ~Player() = default;

    void handleInput();
    void update(const GameContext& context) override ; 
    void draw(sf::RenderWindow& window) override;
};