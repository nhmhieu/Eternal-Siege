#pragma once

#include "Entity.h"
#include <SFML/Graphics.hpp>
#include "TextureManager.h"

class GameContext;

class Ally : public Entity {
private:
    float range = 105.f;
    const sf::Texture* allyTexture = nullptr;
    sf::RectangleShape rectShape;
    Entity* target = nullptr;

public:
    Ally(float x, float y, TextureManager& textureManager, const std::string& textureName);

    void update(GameContext& context) override;
    void draw(sf::RenderWindow& window) override;
    void forgetEntity(const Entity* entity) override;

    Entity* getTarget() const { return target; }
    void updateTarget(const GameContext& context);

    bool isInRange(const Entity* target) const;

    bool canAttack() const {
        if (!target) return false;
        const bool isReady = !isAttacking && coolDownTimer <= 0.f;
        const bool inRange = isInRange(target);
        return inRange && isReady;
    }

    void finishAttack() { isAttacking = false; }

    sf::FloatRect getCollisionBox() const override;
    sf::FloatRect getHurtBox() const override;
};
