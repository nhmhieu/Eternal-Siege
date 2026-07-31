#pragma once

#include <SFML/Graphics.hpp>

#include <algorithm>
#include <memory>
#include <utility>

#include "GameTypes.h"

class GameContext;
class Weapon;

class Entity {
protected:
    // ---------- CORE PROPERTIES ----------
    sf::Texture defaultTexture;
    sf::Sprite sprite;

    sf::Vector2f position{0.f, 0.f};

    float health = 0.f;
    float maxHealth = 0.f;

    Team team = Team::Neutral;
    bool isAlive = true;

    // ---------- MOVEMENT ----------
    sf::Vector2f direction{0.f, 1.f};
    float speed = 0.f;

    // ---------- ATTACK ----------
    sf::Vector2f attackDirection{0.f, 0.f};

    bool isAttacking = false;

    float attackDuration = 1.f;
    float attackPower = 10.f;
    float attackTimer = 0.f;

    float attackCoolDown = 1.f;
    float coolDownTimer = 1.f;

    float gap = 0.f;

    // ---------- WEAPON ----------
    std::unique_ptr<Weapon> currentWeapon;

    // ---------- DEAD ----------
    bool isDying = false;
    float deadTimer = 0.f;
    float deadAnimationDuration = 0.5f;

public:
    // ===============================
    // CONSTRUCTORS & DESTRUCTOR
    // ===============================
    Entity();

    Entity(
        float x,
        float y,
        float health,
        float maxHealth
    );

    Entity(
        float x,
        float y,
        float health,
        float maxHealth,
        Team team,
        std::unique_ptr<Weapon> weapon
    );

    // Không default destructor trực tiếp trong header
    // vì Weapon hiện mới chỉ được forward-declare.
    virtual ~Entity();

    // Không cho phép copy Entity vì Entity sở hữu unique_ptr<Weapon>.
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    // Cho phép move nếu cần.
    Entity(Entity&&) noexcept = default;
    Entity& operator=(Entity&&) noexcept = default;

    // ===============================
    // POSITION
    // ===============================
    float getX() const {
        return position.x;
    }

    float getY() const {
        return position.y;
    }

    sf::Vector2f getPosition() const {
        return position;
    }

    void setPosition(float x, float y) {
        position = {x, y};
        sprite.setPosition(position);
    }

    void setX(float x) {
        setPosition(x, position.y);
    }

    void setY(float y) {
        setPosition(position.x, y);
    }

    // ===============================
    // HEALTH
    // ===============================
    float getHealth() const {
        return health;
    }

    float getMaxHealth() const {
        return maxHealth;
    }

    bool isDead() const {
        return !isAlive;
    }

    void heal(float amount) {
        if (amount <= 0.f || isDead()) {
            return;
        }

        health = std::min(health + amount, maxHealth);
    }

    void increaseMaxHealth(float amount) {
        if (amount <= 0.f || isDead()) {
            return;
        }

        maxHealth += amount;

        // Tăng máu hiện tại cùng lượng với máu tối đa.
        health = std::min(health + amount, maxHealth);
    }

    void decreaseMaxHealth(float amount) {
        if (amount <= 0.f || isDead()) {
            return;
        }

        maxHealth = std::max(1.f, maxHealth - amount);
        health = std::clamp(health - amount, 1.f, maxHealth);
    }

    // ===============================
    // TEAM
    // ===============================
    Team getTeam() const {
        return team;
    }

    // ===============================
    // MOVEMENT
    // ===============================
    sf::Vector2f getDirection() const {
        return direction;
    }

    void setDirection(sf::Vector2f dir) {
        direction = dir;
    }

    // ===============================
    // ATTACK
    // ===============================
    bool getIsAttacking() const {
        return isAttacking;
    }

    void setIsAttacking(bool value) {
        if (value) {
            startAttacking();
        } else {
            isAttacking = false;
            attackTimer = 0.f;
        }
    }

    float getAttackPower() const {
        return attackPower;
    }

    void setAttackPower(float power) {
        attackPower = std::max(0.f, power);
    }

    float getAttackCooldown() const {
        return attackCoolDown;
    }

    void setAttackCooldown(float seconds) {
        attackCoolDown = std::max(0.15f, seconds);

        // Nếu cooldown đang chờ dài hơn cooldown mới,
        // rút timer về bằng cooldown mới.
        coolDownTimer = std::min(
            coolDownTimer,
            attackCoolDown
        );
    }

    sf::Vector2f getAttackDirection() const {
        return attackDirection;
    }

    void setAttackDirection(sf::Vector2f dir) {
        attackDirection = dir;
    }

    // ===============================
    // WEAPON
    // ===============================
    Weapon* getCurrentWeapon() const {
        return currentWeapon.get();
    }

    // Định nghĩa trong Entity.cpp vì cần Weapon là complete type.
    void setCurrentWeapon(std::unique_ptr<Weapon> weapon);

    // ===============================
    // DEAD STATE
    // ===============================
    bool getIsDying() const {
        return isDying;
    }

    void setIsDying(bool status) {
        isDying = status;
    }

    void startDying() {
        if (isDying) {
            return;
        }

        isDying = true;
        deadTimer = 0.f;
    }

    bool isReadyToBeDelete() const {
        return isDying &&
               deadTimer >= deadAnimationDuration;
    }

    // ===============================
    // CORE LOGIC
    // ===============================
    virtual void takeDamage(float damage);

    virtual void update(GameContext& context);
    virtual void forgetEntity(const Entity* entity);

    virtual void draw(sf::RenderWindow& window) = 0;

    // ===============================
    // COLLISION & HITBOX
    // ===============================
    virtual sf::FloatRect getCollisionBox() const;

    virtual sf::FloatRect getHurtBox() const;

    virtual sf::FloatRect getAttackHitbox() const;

    // ===============================
    // ATTACK STATE
    // ===============================
    void updateStatus();

    void updateAttackTimer(
        const GameContext& context
    );

    void startAttacking() {
        if (isDead() || isAttacking) {
            return;
        }

        isAttacking = true;
        attackTimer = 0.f;
    }

    // ===============================
    // HEALTH BAR
    // ===============================
    virtual void drawHealthBar(
        sf::RenderWindow& window
    ) const;

    // ===============================
    // DEAD TIMER
    // ===============================
    void updateDeadTimer(
        const GameContext& context
    );
};
