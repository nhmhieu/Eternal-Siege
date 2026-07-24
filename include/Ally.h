#pragma once

#include "Entity.h"
#include <SFML/Graphics.hpp>
#include "TextureManager.h"

class GameContext;

class Ally : public Entity {
private:
    float attackRange = 200.f;
    float attackCoolDown = 1.5f;
    float cooldownTimer = 0.f;
    const sf::Texture* allyTexture;
    sf::RectangleShape rectShape;
public:
    Ally(float x, float y);
    Ally(float x, float y, TextureManager& textureManager, const std::string& textureName);

    void update(const GameContext& context) override;
    void draw(sf::RenderWindow& window) override;

    sf::FloatRect getCollisionBox() const override;
    sf::FloatRect getHurtBox() const override;
};