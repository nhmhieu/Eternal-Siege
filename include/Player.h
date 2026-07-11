#pragma once
#include <SFML/Graphics.hpp>

class Player{
private:
    sf::CircleShape sprite;
    float speed;

public:
    Player();
    ~Player() = default;

    void handleInput();
    void update(float dt, const sf::RenderWindow& window);
    void render(sf::RenderWindow& window);
};