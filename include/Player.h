#pragma once

#include "Entity.h"
#include <SFML/Graphics.hpp>

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
};