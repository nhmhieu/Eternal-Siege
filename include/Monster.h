#pragma once

#include "Entity.h"
#include <vector>

class GameContext;
class Map;
class Effects;

class Monster : public Entity {
protected:
    float attackRange = 45.f;
    float moveSpeed = 90.f;
    Entity* currentTarget = nullptr;
    sf::RectangleShape monsterShape;
    const sf::Texture* monsterTexture = nullptr;
    sf::Vector2f collisionSize{32.f, 32.f};
    sf::Vector2f hurtBoxSize{32.f, 32.f};
    sf::Vector2f visualSize{32.f, 32.f};
    sf::Vector2f previousPosition;
    float footstepDistance = 0.f;
    float walkDistanceAccumulator = 0.f;
    float facingDirection = 1.f;
    int animDirectionRow = 0;
    int animFrameIndex = 0;
    bool isSpriteSheet = false;
    sf::Vector2i frameSize{256, 384};
    bool visuallyMoving = false;
    bool eliteVisual = false;
    sf::Color presentationTint{205, 65, 65};

private:
    std::vector<sf::Vector2i> path;
    std::size_t waypointIndex = 0;
    sf::Vector2i lastGoalCell{-999, -999};
    float pathRefreshTimer = 0.f;
    int goldReward = 10;
    bool rewardClaimed = false;

    void rebuildPath(const Map& map);
    void followPath(float deltaTime, const Map& map);
    void updatePresentation(Effects* effects);

public:
    Monster(float x, float y, float health = 100.f, float maxHealth = 100.f,
            float range = 45.f, float cooldown = 1.f,
            float speed = 90.f, float damage = 10.f);
    ~Monster() override = default;

    void updateTarget(const std::vector<Entity*>& targets);
    void update(GameContext& context) override;
    void draw(sf::RenderWindow& window) override;
    void drawShadow(sf::RenderWindow& window) const;
    void forgetEntity(const Entity* entity) override;
    void setPosition(float x, float y);
    void moveWithCollision(sf::Vector2f displacement, const Map& map);
    void setPresentationTexture(
        const sf::Texture* texture,
        sf::IntRect visibleBounds,
        float desiredHeight,
        sf::Color tint = sf::Color::White);

    sf::FloatRect getCollisionBox() const override;
    sf::FloatRect getHurtBox() const override;
    Entity* getCurrentTarget() const { return currentTarget; }
    float getMoveSpeed() const { return moveSpeed; }
    sf::Vector2f getVisualSize() const { return visualSize; }
    sf::Vector2f getCollisionSize() const { return collisionSize; }
    sf::Vector2f getHurtBoxSize() const { return hurtBoxSize; }

    virtual bool isBoss() const { return false; }
    virtual bool isElite() const { return false; }
    int claimGoldReward();
    void setGoldReward(int value) { goldReward = value; }
};
