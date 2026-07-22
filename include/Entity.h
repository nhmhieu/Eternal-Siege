#pragma once

#include <SFML/Graphics/Sprite.hpp>
#include <vector>
#include "Weapon.h" 
#include <memory>
// ===============================
// 1. ENUM
// ===============================
enum class Team {
    Player,
    Enemy,
    Neutral
};

class GameContext;

// ===============================
// 2. LỚP ENTITY
// ===============================
class Entity {
protected:
    // ---------- CORE PROPERTIES ----------
    sf::Texture defaultTexture;
    sf::Sprite sprite;   
    sf::Vector2f position;
    float health;
    float maxHealth;
    Team team;
    bool isAlive = true;   

    // ---------- MOVEMENT ----------
    sf::Vector2f direction = { 0.f, 1.f }; // Hướng di chuyển
    float speed = 0.f;

    // ---------- ATTACK ----------
    sf::Vector2f attackDirection = { 0.f, 0.f };
    bool isAttacking = false;
    float attackDuration = 0.2f;
    float attackPower = 10.f;
    sf::Clock attackClock;

    // ---------- WEAPON ----------
    Weapon* currentWeapon = nullptr;

public:
    // ===============================
    // 3. CONSTRUCTORS & DESTRUCTOR
    // ===============================
    Entity();
    Entity(float x, float y, float health, float maxHealth);
    Entity(float x, float y, float health, float maxHealth, Team team, Weapon* weapon);
    virtual ~Entity() = default;

    // ===============================
    // 4. GETTERS & SETTERS
    // ===============================
    // Position
    float getX() const { return position.x; }
    float getY() const { return position.y; }
    sf::Vector2f getPosition() const { return position; }
    void setPosition(float x, float y) { position = { x, y }; sprite.setPosition(position); }
    void setX(float x) { position.x = x; sprite.setPosition(position); }
    void setY(float y) { position.y = y; sprite.setPosition(position); }
    // Health
    float getHealth() const { return health; }
    float getMaxHealth() const { return maxHealth; }
    bool isDead() const { return !isAlive; }  // ← ĐÃ SỬA LỖI

    // Team
    Team getTeam() const { return team; }

    // Movement
    sf::Vector2f getDirection() const { return direction; }
    void setDirection(sf::Vector2f dir) { direction = dir; }

    // Attack
    bool getIsAttacking() const { return isAttacking; }
    void setIsAttacking(bool val) { isAttacking = val; if (val) attackClock.restart(); }
    float getAttackPower() const { return attackPower; }
    void setAttackPower(float power) { attackPower = power; }
    sf::Vector2f getAttackDirection() const { return attackDirection; }
    void setAttackDirection(sf::Vector2f dir) { attackDirection = dir; }

    // Weapon
    Weapon* getCurrentWeapon() const { return currentWeapon; }
    void setCurrentWeapon(Weapon* weapon) { currentWeapon = weapon; }

    // ===============================
    // 5. CORE LOGIC
    // ===============================
    virtual void takeDamage(float damage);
    virtual void update(const GameContext& context) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;

    // ===============================
    // 6. COLLISION & HITBOX HELPERS
    // ===============================
    virtual sf::FloatRect getCollisionBox() const;
    virtual sf::FloatRect getHurtBox() const;
    virtual sf::FloatRect getAttackHitbox() const;

    // ===============================
    // 7. ATTACK STATE MANAGEMENT
    // ===============================
    void updateStatus(); // Cập nhật trạng thái tấn công
};