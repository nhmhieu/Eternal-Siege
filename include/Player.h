#pragma once

#include "Entity.h"
#include <SFML/Graphics.hpp>
#include "TextureManager.h"

class GameContext;

class Player : public Entity {
private:
    float speed = 300.f;
    const sf::Texture* playerTexture;
    //sf::RectangleShape playerShape;
    std::unique_ptr<sf::Sprite> sprite;
    sf::RectangleShape fallbackShape;
    bool useFallback = false;
    sf::CircleShape debugDot;  // Debug: chấm đỏ ở vị trí entity
public:
    Player();

    void handleInput();
    void update(const GameContext& context) override;
    void draw(sf::RenderWindow& window) override;

    sf::FloatRect getCollisionBox() const override;
    sf::FloatRect getHurtBox() const override;
};