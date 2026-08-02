#include "Entity.h"

#include "GameContext.h"
#include "Weapon.h"

#include <algorithm>
#include <utility>

// ===============================
// CONSTRUCTORS & DESTRUCTOR
// ===============================

Entity::Entity()
    : sprite(defaultTexture),
      position(0.f, 0.f),
      health(0.f),
      maxHealth(0.f) {
    sprite.setPosition(position);
}

Entity::Entity(
    float x,
    float y,
    float initialHealth,
    float initialMaxHealth
)
    : sprite(defaultTexture),
      position(x, y),
      health(initialHealth),
      maxHealth(initialMaxHealth) {
    sprite.setPosition(position);
}

Entity::Entity(
    float x,
    float y,
    float initialHealth,
    float initialMaxHealth,
    Team entityTeam,
    std::unique_ptr<Weapon> weapon
)
    : sprite(defaultTexture),
      position(x, y),
      health(initialHealth),
      maxHealth(initialMaxHealth),
      team(entityTeam),
      currentWeapon(std::move(weapon)) {
    sprite.setPosition(position);
}

// Weapon đã là complete type trong Entity.cpp
// vì phía trên có #include "Weapon.h".
Entity::~Entity() = default;

// ===============================
// WEAPON
// ===============================

void Entity::setCurrentWeapon(
    std::unique_ptr<Weapon> weapon
) {
    currentWeapon = std::move(weapon);
}

void Entity::setIsAttacking(bool value) {
    if (value) {
        startAttacking();
        return;
    }

    isAttacking = false;
    attackTimer = 0.f;
    if (currentWeapon) {
        currentWeapon->clearHitList();
        currentWeapon->setHasAttacked(false);
    }
}

void Entity::beginAttackCooldown() {
    setIsAttacking(false);
    coolDownTimer = attackCoolDown;
}

void Entity::forgetEntity(const Entity* entity) {
    if (currentWeapon) {
        currentWeapon->forgetEntity(entity);
    }
}

// ===============================
// HEALTH
// ===============================

void Entity::takeDamage(float damage) {
    // Không nhận sát thương âm, và entity chết rồi
    // thì không tiếp tục bị trừ máu.
    if (damage <= 0.f || isDead()) {
        return;
    }

    hurtFlashTimer = 0.11f;
    health = std::max(0.f, health - damage);

    if (health <= 0.f) {
        isAlive = false;
        setIsAttacking(false);

        // Bắt đầu đếm thời gian animation chết.
        startDying();
    }
}

// ===============================
// COLLISION & HITBOX
// ===============================

sf::FloatRect Entity::getCollisionBox() const {
    return sprite.getGlobalBounds();
}

sf::FloatRect Entity::getHurtBox() const {
    return sprite.getGlobalBounds();
}

sf::FloatRect Entity::getAttackHitbox() const {
    const sf::FloatRect bounds = sprite.getGlobalBounds();

    const sf::Vector2f center =
        bounds.position + bounds.size / 2.f;

    constexpr float attackRange = 40.f;

    const sf::Vector2f hitboxSize(40.f, 40.f);

    const sf::Vector2f attackCenter =
        center + attackDirection * attackRange;

    const sf::Vector2f attackPosition =
        attackCenter - hitboxSize / 2.f;

    return sf::FloatRect(
        attackPosition,
        hitboxSize
    );
}

// ===============================
// ATTACK STATE
// ===============================

void Entity::updateAttackTimer(
    const GameContext& context
) {
    if (!isAttacking) {
        return;
    }

    attackTimer += context.deltaTime;
}

void Entity::updateStatus() {
    if (!isAttacking) {
        return;
    }

    if (attackTimer < attackDuration) {
        return;
    }

    // Kết thúc một lần tấn công.
    isAttacking = false;
    attackTimer = 0.f;

    // Bắt đầu thời gian chờ cho lần đánh tiếp theo.
    coolDownTimer = attackCoolDown;

    if (currentWeapon) {
        // Cho phép weapon đánh lại entity trong đòn tiếp theo.
        currentWeapon->clearHitList();

        // Cho phép weapon thực hiện action trong đòn tiếp theo.
        currentWeapon->setHasAttacked(false);
    }
}

float Entity::getAttackAnimationProgress() const {
    if (!isAttacking || attackDuration <= 0.f) {
        return 0.f;
    }
    return std::clamp(attackTimer / attackDuration, 0.f, 1.f);
}

// ===============================
// DEAD TIMER
// ===============================

void Entity::updateDeadTimer(
    const GameContext& context
) {
    if (!isDying) {
        return;
    }

    deadTimer += context.deltaTime;
}

// ===============================
// BASE UPDATE
// ===============================

void Entity::update(GameContext& context) {
    if (context.paused) {
        return;
    }

    visualTime += context.deltaTime;
    hurtFlashTimer = std::max(0.f, hurtFlashTimer - context.deltaTime);
    healingFlashTimer = std::max(
        0.f, healingFlashTimer - context.deltaTime);

    // Entity lớp con gọi Entity::update(context)
    // để cập nhật cooldown dùng chung.
    if (coolDownTimer > 0.f) {
        coolDownTimer -= context.deltaTime;

        if (coolDownTimer < 0.f) {
            coolDownTimer = 0.f;
        }
    }
}

// ===============================
// HEALTH BAR
// ===============================

void Entity::drawHealthBar(
    sf::RenderWindow& window
) const {
    constexpr float barHeight = 6.f;

    const sf::Vector2f barPosition(
        position.x - healthBarWidth / 2.f,
        position.y + healthBarOffsetY
    );

    // Nền thanh máu.
    sf::RectangleShape background(
        sf::Vector2f(healthBarWidth, barHeight)
    );

    background.setFillColor(
        sf::Color(60, 60, 60)
    );

    background.setPosition(barPosition);
    window.draw(background);

    float healthPercent = 0.f;

    if (maxHealth > 0.f) {
        healthPercent = health / maxHealth;
    }

    // Bảo đảm tỉ lệ luôn nằm trong [0, 1].
    healthPercent = std::clamp(
        healthPercent,
        0.f,
        1.f
    );

    sf::RectangleShape healthBar(
        sf::Vector2f(
            healthBarWidth * healthPercent,
            barHeight
        )
    );

    if (team == Team::Enemy) {
        healthBar.setFillColor(sf::Color::Red);
    } else {
        healthBar.setFillColor(sf::Color::Green);
    }

    healthBar.setPosition(barPosition);
    window.draw(healthBar);
}
