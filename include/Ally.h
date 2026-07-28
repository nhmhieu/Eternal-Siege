#pragma once

#include "Entity.h"
#include <SFML/Graphics.hpp>
#include "TextureManager.h"

class GameContext;

class Ally : public Entity {
private:
    float range = 200.f;
    float attackCoolDown = 2.f;
    float coolDownTimer = 0.f;
    const sf::Texture* allyTexture;
    sf::RectangleShape rectShape;
    Entity* target = nullptr;

public:
    Ally(float x, float y);
    Ally(float x, float y, TextureManager& textureManager, const std::string& textureName);

    void update(GameContext& context) override;
    void draw(sf::RenderWindow& window) override;

    Entity* getTarget() const { return target; }
    void updateTarget(const GameContext& context);

    bool isInRange(Entity* target) ;

    bool canAttack() {
        if (!target) return false;
        bool isReady = !isAttacking && coolDownTimer <= 0;
        bool inRange = isInRange(target);
        return inRange && isReady;
    }

    void finishAttack() { isAttacking = false; }

    sf::FloatRect getCollisionBox() const override;
    sf::FloatRect getHurtBox() const override;
};