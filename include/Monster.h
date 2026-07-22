#pragma once

#include "Entity.h"
#include <vector>

class GameContext;

class Monster : public Entity {
private:
    float attackRange = 50.f;
    float attackCooldown = 1.f;
    float attackDamage = 10.f;
    float speed = 100.f;
    Entity* currentTarget = nullptr;
    float targetTimer = 0.f;
    float attackTimer = 0.f;
    sf::RectangleShape fallbackShape;
    bool useFallback = true;  // Monster không có texture → luôn dùng fallback

public:
    Monster(float x, float y, float health = 100, float maxHealth = 100,
        float range = 50.f, float cooldown = 1.f, float spd = 100.f, float dmg = 10.f);
    ~Monster() override = default;

    void updateTarget(const std::vector<Entity*>& targets);
    void moveToward(float deltaTime);

     void update(const GameContext& context) override;
     void draw(sf::RenderWindow& window) override;

     sf::FloatRect getCollisionBox() const override;
     sf::FloatRect getHurtBox() const override;

     Entity* getCurrentTarget() const { return currentTarget; }
};
