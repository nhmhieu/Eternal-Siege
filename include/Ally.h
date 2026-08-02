#pragma once

#include "AllyConfig.h"
#include "Entity.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include "TextureManager.h"

class GameContext;
class Effects;

class Ally : public Entity {
private:
    AllyType type;
    const sf::Texture* allyTexture = nullptr;
    sf::RectangleShape rectShape;
    sf::CircleShape skillEffectShape;
    Entity* target = nullptr;
    float skillCooldownRemaining = 0.f;
    float skillEffectRemaining = 0.f;
    bool skillActive = false;
    sf::Vector2f previousPosition;
    float footstepDistance = 0.f;

    void updateSkillTimers(float deltaTime);
    bool tryUseSkill(GameContext& context);
    void activateSkillVisual(float radius, sf::Color color);
    void startSkillCooldown();
    int calculateSkillDamage(float multiplier) const;
    int countEnemiesInRange(const GameContext& context, float range) const;
    Entity* findClosestEnemyInRange(
        const GameContext& context, float range) const;
    int damageEnemiesInRange(
        const GameContext& context, float range, float multiplier,
        bool strongEffect);
    bool castExplosiveArrow(GameContext& context);
    bool castArcaneBurst(GameContext& context);
    bool castWhirlwind(GameContext& context);
    void activateShieldWall(GameContext& context);
    void updatePresentation(Effects* effects);

public:
    Ally(float x, float y, TextureManager& textureManager, AllyType allyType);
    Ally(
        float x,
        float y,
        TextureManager& textureManager,
        const std::string& allyName
    );

    void update(GameContext& context) override;
    void draw(sf::RenderWindow& window) override;
    void drawShadow(sf::RenderWindow& window) const;
    void forgetEntity(const Entity* entity) override;
    void takeDamage(float damage) override;

    Entity* getTarget() const { return target; }
    AllyType getType() const { return type; }
    AllyType getAllyType() const { return type; }
    const char* getSkillName() const;
    float getSkillCooldownRemaining() const {
        return skillCooldownRemaining;
    }
    float getSkillCooldownDuration() const;
    bool isSkillActive() const { return skillActive; }
    float getAttackRange() const;
    void updateTarget(const GameContext& context);

    bool isInRange(const Entity* target) const;

    bool canAttack() const {
        if (isDead() || !target || target->isDead() || !currentWeapon) {
            return false;
        }
        const bool isReady = !isAttacking && coolDownTimer <= 0.f;
        const bool inRange = isInRange(target);
        return inRange && isReady;
    }

    sf::FloatRect getCollisionBox() const override;
    sf::FloatRect getHurtBox() const override;
};

std::unique_ptr<Ally> createAlly(
    AllyType type,
    sf::Vector2f position,
    TextureManager& textureManager
);
