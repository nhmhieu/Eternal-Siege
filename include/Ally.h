#pragma once
#include "Entity.h"
#include "TextureManager.h"

class Ally : public Entity {
private:
    const sf::Texture* allyTexture;
    float attackRange = 250.f;
    float attackCoolDown = 0.8f;
    float cooldownTimer = 0.f;
    float speed = 160.f;
    sf::RectangleShape fallbackShape;
    sf::RectangleShape rectShape;
    bool useFallback = false;
    sf::RectangleShape debugRect;
public:
    Ally(float x, float y);
    Ally(float x, float y, TextureManager& textureManager, const std::string& textureName);
    ~Ally() override = default;
    
     void update(const GameContext& context) override;
     void draw(sf::RenderWindow& window) override;

     sf::FloatRect getCollisionBox() const override;
     sf::FloatRect getHurtBox() const override;

     bool isUsingFallback() const { return useFallback; }
};
