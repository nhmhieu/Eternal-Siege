#pragma once
#include <SFML/Graphics.hpp>
#include "TextureManager.h"
#include <memory>

class Player {
private:
    //sf::CircleShape sprite;
    std::unique_ptr<sf::Sprite> sprite;
    float speed;

public:
    Player();
    ~Player() = default;

    void handleInput();
    void update(float dt, const sf::RenderWindow& window);
    void render(sf::RenderWindow& window);
};