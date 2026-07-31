#pragma once

#include "Entity.h"
#include <vector>

class GameContext;
class Map;

class Monster : public Entity {
protected:
    float attackRange = 45.f;
    float moveSpeed = 90.f;
    Entity* currentTarget = nullptr;
    sf::RectangleShape monsterShape;

private:
    std::vector<sf::Vector2i> path;
    std::size_t waypointIndex = 0;
    sf::Vector2i lastGoalCell{-999, -999};
    float pathRefreshTimer = 0.f;
    int goldReward = 10;
    bool rewardClaimed = false;

    void rebuildPath(const Map& map);
    void followPath(float deltaTime, const Map& map);

public:
    Monster(float x, float y, float health = 100.f, float maxHealth = 100.f,
            float range = 45.f, float cooldown = 1.f,
            float speed = 90.f, float damage = 10.f);
    ~Monster() override = default;

    void updateTarget(const std::vector<Entity*>& targets);
    void update(GameContext& context) override;
    void draw(sf::RenderWindow& window) override;
    void forgetEntity(const Entity* entity) override;
    void setPosition(float x, float y);
    void moveWithCollision(sf::Vector2f displacement, const Map& map);

    sf::FloatRect getCollisionBox() const override;
    sf::FloatRect getHurtBox() const override;
    Entity* getCurrentTarget() const { return currentTarget; }

    virtual bool isBoss() const { return false; }
    virtual bool isElite() const { return false; }
    int claimGoldReward();
    void setGoldReward(int value) { goldReward = value; }
};
