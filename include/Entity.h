#pragma once

#include <SFML/Graphics.hpp>      
#include <memory>                 // Cho unique_ptr
#include <vector>
#include <iostream>
#include <SFML/System/Clock.hpp>
#include "GameTypes.h"

class GameContext;
class Weapon;

class Entity {
protected:
    // ---------- CORE PROPERTIES ----------
    sf::Texture defaultTexture;
    sf::Sprite sprite;
    sf::Vector2f position;
    float health;
    float maxHealth;
    Team team = Team::Neutral;
    bool isAlive = true;

    // ---------- MOVEMENT ----------
    sf::Vector2f direction = { 0.f, 1.f };
    float speed = 0.f;

    // ---------- ATTACK ----------
    sf::Vector2f attackDirection = { 0.f, 0.f };
    bool isAttacking = false;
    float attackDuration = 1.0f;
    float attackPower = 10.f;
    sf::Clock attackClock;
    float attackTimer = 0.f;
    float attackCoolDown = 1.f;
    float coolDownTimer = attackCoolDown;
    float gap = 0;

    // ---------- WEAPON ----------
    std::unique_ptr<Weapon> currentWeapon;   // Sử dụng unique_ptr để rõ ràng ownership

    // ---------- DEAD ----------
    bool isDying = false;
    float deadTimer = 0.f;
    float deadAnimationDuration = 0.5f;

public:
    // ===============================
    // CONSTRUCTORS & DESTRUCTOR
    // ===============================
    Entity();
    Entity(float x, float y, float health, float maxHealth);
    Entity(float x, float y, float health, float maxHealth, Team team, std::unique_ptr<Weapon> weapon);
    virtual ~Entity() = default;   // unique_ptr tự động giải phóng

    // ===============================
    // GETTERS & SETTERS
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
    bool isDead() const { return !isAlive; }

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
    void setAttackCooldown(float seconds) {
        attackCoolDown = seconds;
        coolDownTimer = 0.f;
    }
    sf::Vector2f getAttackDirection() const { return attackDirection; }
    void setAttackDirection(sf::Vector2f dir) { attackDirection = dir; }

    // Weapon
    Weapon* getCurrentWeapon() const { return currentWeapon.get(); }
    void setCurrentWeapon(std::unique_ptr<Weapon> weapon) { currentWeapon = std::move(weapon); }

    // Dead state
    bool getIsDying() const { return isDying; }
    void setIsDying(bool status) { isDying = status; }
    void startDying() { isDying = true; }
    bool isReadyToBeDelete() const { return isDying && deadTimer >= deadAnimationDuration; }

    // ===============================
    // CORE LOGIC
    // ===============================
    virtual void takeDamage(float damage);
    virtual void update(GameContext& context);
    virtual void draw(sf::RenderWindow& window) = 0;

    // ===============================
    // COLLISION & HITBOX HELPERS
    // ===============================
    virtual sf::FloatRect getCollisionBox() const;
    virtual sf::FloatRect getHurtBox() const;
    virtual sf::FloatRect getAttackHitbox() const;

    // ===============================
    // ATTACK STATE MANAGEMENT
    // ===============================
    void updateStatus();
    void updateAttackTimer(const GameContext& context);
    void startAttacking() { isAttacking = true; }

    // ===============================
    // HEALTH BAR VISUALIZATION
    // ===============================
    virtual void drawHealthBar(sf::RenderWindow& window) const;

    // ===============================
    // DEAD TIMER UPDATE
    // ===============================
    void updateDeadTimer(const GameContext& context);
};

 
