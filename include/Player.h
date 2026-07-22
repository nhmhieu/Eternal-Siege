#pragma once

#include "Entity.h"
#include <SFML/Graphics.hpp>
#include <TextureManager.h>

class GameContext;

class Player : public Entity {
private:
    float speed = 300.f;
public:
    Player(TextureManager& textureManager);

    void handleInput();
    void update(const GameContext& context) override;
    void draw(sf::RenderWindow& window) override;
};