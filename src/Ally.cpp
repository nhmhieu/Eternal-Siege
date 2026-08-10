#include "Ally.h"
#include "BalanceConfig.h"
#include "Bow.h"
#include "GameContext.h"
#include "Projectiles.h"
#include "Sword.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <memory>
#include "TextureManager.h"
#include "Weapon.h"
#include "Wand.h"
#include "Effects.h"
#include "AudioManager.h"

namespace {
const std::array<AllyStats, 4> ALLY_STATS{{
    {450.f, 28, 1.75f, 250.f, WeaponType::Bow, 1.f},
    {400.f, 24, 2.00f, 115.f, WeaponType::Sword, 0.75f},
    {324.f, 17, 1.05f, 220.f, WeaponType::Wand, 1.f},
    {360.f, 38, 2.70f, 135.f, WeaponType::Sword, 1.f}
}};

const std::array<AllySkillConfig, 4> ALLY_SKILLS{{
    {AllySkillType::ExplosiveArrow, "Explosive Arrow",
     9.f, 250.f, 0.25f, 1.25f, 0.75f, 90.f, 1},
    {AllySkillType::ShieldWall, "Shield Wall",
     12.f, 0.f, 4.f, 0.f, 0.f, 0.f, 0},
    {AllySkillType::ArcaneBurst, "Arcane Burst",
     7.f, 170.f, 0.25f, 1.40f, 0.f, 0.f, 2},
    {AllySkillType::Whirlwind, "Whirlwind",
     9.f, 120.f, 0.25f, 1.10f, 0.f, 0.f, 2}
}};

std::size_t allyIndex(AllyType type) {
    return static_cast<std::size_t>(type);
}
}

const AllyStats& getAllyStats(AllyType type) {
    return ALLY_STATS[allyIndex(type)];
}

const AllySkillConfig& getAllySkillConfig(AllyType type) {
    return ALLY_SKILLS[allyIndex(type)];
}

const char* getAllyName(AllyType type) {
    switch (type) {
    case AllyType::Damian:     return "Damian";
    case AllyType::Evangeline: return "Evangeline";
    case AllyType::Junior:     return "Junior";
    case AllyType::Lucas:      return "Lucas";
    }
    return "Damian";
}

const char* getAllyRole(AllyType type) {
    switch (type) {
    case AllyType::Damian:     return "Ranged Striker";
    case AllyType::Evangeline: return "Guardian";
    case AllyType::Junior:     return "Arcane Support";
    case AllyType::Lucas:      return "Frontline Bruiser";
    }
    return "Ally";
}

const char* getWeaponName(WeaponType type) {
    switch (type) {
    case WeaponType::Sword: return "Sword";
    case WeaponType::Bow:   return "Bow";
    case WeaponType::Wand:  return "Wand / Magic";
    }
    return "Weapon";
}

AllyType getAllyType(std::string_view name) {
    if (name == "Evangeline") return AllyType::Evangeline;
    if (name == "Junior") return AllyType::Junior;
    if (name == "Lucas") return AllyType::Lucas;
    return AllyType::Damian;
}

// ===============================
// CONSTRUCTORS
// ===============================


Ally::Ally(
    float x,
    float y,
    TextureManager& textureManager,
    AllyType allyType
)
    : Entity(
          x,
          y,
          getAllyStats(allyType).maxHealth,
          getAllyStats(allyType).maxHealth
      ),
      type(allyType),
      allyTexture(nullptr) {
    const AllyStats& stats = getAllyStats(type);
    team = Team::Player;
    attackPower = static_cast<float>(stats.attackDamage);
    attackCoolDown = stats.attackCooldown;
    coolDownTimer = 0.f;

    allyTexture = textureManager.findTexture(getAllyName(type));

    const float desiredSize = 70.f;
    const sf::IntRect visible =
        textureManager.getVisibleBounds(getAllyName(type));

    if (allyTexture && visible.size.x > 0 && visible.size.y > 0) {
        const float aspect = static_cast<float>(visible.size.x) /
                             static_cast<float>(visible.size.y);
        const sf::Vector2f displaySize = aspect >= 1.f
            ? sf::Vector2f{desiredSize, desiredSize / aspect}
            : sf::Vector2f{desiredSize * aspect, desiredSize};
        rectShape.setSize(displaySize);
        rectShape.setTexture(allyTexture);
        rectShape.setTextureRect(visible);
        rectShape.setOrigin(displaySize / 2.f);
    }
    else {
        constexpr float fallbackSize = 40.f;
        rectShape.setSize({fallbackSize, fallbackSize});
        rectShape.setOrigin({fallbackSize / 2.f, fallbackSize / 2.f});
        rectShape.setFillColor(sf::Color::Cyan);
    }
    rectShape.setPosition(sf::Vector2f(x, y));
    previousPosition = {x, y};

    if (stats.weaponType == WeaponType::Sword) {
        setCurrentWeapon(std::make_unique<Sword>(stats.attackRange));
    } else if (stats.weaponType == WeaponType::Bow) {
        setCurrentWeapon(std::make_unique<Bow>());
    } else {
        setCurrentWeapon(std::make_unique<Wand>(stats.attackRange));
    }
}

Ally::Ally(
    float x,
    float y,
    TextureManager& textureManager,
    const std::string& allyName
)
    : Ally(x, y, textureManager, ::getAllyType(allyName)) {
}

// ===============================
// CORE LOGIC
// ===============================

void Ally::update(GameContext& context) {
    if (context.paused) return;
    Entity::update(context);

    if (isDead()) {
        target = nullptr;
        setIsAttacking(false);
        skillActive = false;
        skillEffectRemaining = 0.f;
        if (!isDying) startDying();
        updateDeadTimer(context);
        updatePresentation(context.effects);
        return;
    }

    // 1. Cập nhật mục tiêu
    updateSkillTimers(context.deltaTime);
    updateTarget(context);

    if (tryUseSkill(context)) {
        updatePresentation(context.effects);
        return;
    }

    // 2. Nếu có mục tiêu hợp lệ
    if (target && !target->isDead()) {
        sf::Vector2f dir = target->getPosition() - this->getPosition();
        float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (length != 0.f) {
            dir /= length;
        }
        else {
            dir = { 1.f, 0.f };
        }
        this->setAttackDirection(dir);

        if (currentWeapon && isInRange(target) && coolDownTimer <= 0.f) {
            startAttacking();
        }
    }

    if (isAttacking) {
        updateAttackTimer(context);
        if (currentWeapon && context.combatManager) {
            currentWeapon->triggerAction(this, context, *context.combatManager);
        }
    }

    updateStatus();

    updatePresentation(context.effects);
}

void Ally::draw(sf::RenderWindow& window) {
    if (skillActive) {
        skillEffectShape.setPosition(getPosition());
        window.draw(skillEffectShape);
    }
    window.draw(rectShape);
}

void Ally::drawShadow(sf::RenderWindow& window) const {
    const float radius = std::max(10.f, rectShape.getSize().x * 0.22f);
    sf::CircleShape shadow(radius);
    shadow.setOrigin({radius, radius});
    shadow.setScale({1.25f, 0.34f});
    shadow.setPosition({position.x, position.y + 15.f});
    shadow.setFillColor(sf::Color(8, 5, 15, isDying ? 35 : 88));
    window.draw(shadow);
}

void Ally::updatePresentation(Effects* effects) {
    const sf::Vector2f moved = position - previousPosition;
    previousPosition = position;
    const float movedDistance = std::sqrt(
        moved.x * moved.x + moved.y * moved.y);
    if (movedDistance > 0.001f && !isDying) {
        footstepDistance += movedDistance;
        if (footstepDistance >= 22.f) {
            footstepDistance = std::fmod(footstepDistance, 22.f);
            if (effects) {
                effects->spawnFootstep(
                    {position.x, position.y + 15.f}, FootstepStyle::Ally);
            }
        }
    }
    const float idleBob =
        std::sin(visualTime * 2.8f + position.x * 0.01f) * 0.9f;
    sf::Vector2f visualPosition = position - sf::Vector2f(0.f, idleBob);
    float scaleX = 1.f;
    float scaleY = 1.f;
    float rotation =
        std::sin(visualTime * 1.8f + position.y * 0.01f) * 0.45f;

    if (isAttacking) {
        const float pulse = std::sin(
            getAttackAnimationProgress() * 3.14159265358979323846f);
        const bool melee = getAttackRange() < 160.f;
        visualPosition += attackDirection * (melee ? 6.f : -4.f) * pulse;
        scaleX += 0.09f * pulse;
        scaleY -= 0.06f * pulse;
        rotation += attackDirection.x * (melee ? 8.f : -4.f) * pulse;
    }

    const float facing = attackDirection.x < -0.05f ? -1.f : 1.f;
    sf::Color tint = allyTexture ? sf::Color::White : sf::Color::Cyan;
    if (isDying) {
        const float progress = std::clamp(
            deadTimer / std::max(0.01f, deadAnimationDuration), 0.f, 1.f);
        rotation += facing * 76.f * progress;
        scaleX *= 1.f - progress * 0.25f;
        scaleY *= 1.f - progress * 0.45f;
        tint.a = static_cast<std::uint8_t>(255.f * (1.f - progress));
    } else if (healingFlashTimer > 0.f) {
        tint = sf::Color(135, 255, 195);
    } else if (hurtFlashTimer > 0.f) {
        tint = sf::Color(255, 135, 135);
    }

    rectShape.setPosition(visualPosition);
    rectShape.setScale({facing * scaleX, scaleY});
    rectShape.setRotation(sf::degrees(rotation));
    rectShape.setFillColor(tint);
}

void Ally::takeDamage(float damage) {
    damage *= getAllyStats(type).damageTakenMultiplier;
    if (type == AllyType::Evangeline && skillActive && !isDead()) {
        damage *= 0.55f;
    }
    Entity::takeDamage(damage);
}

const char* Ally::getSkillName() const {
    return getAllySkillConfig(type).name;
}

float Ally::getSkillCooldownDuration() const {
    return getAllySkillConfig(type).cooldown;
}

void Ally::updateSkillTimers(float deltaTime) {
    skillCooldownRemaining = std::max(
        0.f, skillCooldownRemaining - deltaTime);
    if (!skillActive) return;

    skillEffectRemaining = std::max(
        0.f, skillEffectRemaining - deltaTime);
    if (skillEffectRemaining <= 0.f) {
        skillActive = false;
    }
}

void Ally::activateSkillVisual(float radius, sf::Color color) {
    skillActive = true;
    skillEffectRemaining = getAllySkillConfig(type).effectDuration;
    skillEffectShape.setRadius(radius);
    skillEffectShape.setOrigin({radius, radius});
    skillEffectShape.setFillColor(color);
    skillEffectShape.setOutlineColor(sf::Color(
        color.r, color.g, color.b, 210));
    skillEffectShape.setOutlineThickness(2.f);
    skillEffectShape.setPosition(getPosition());
}

void Ally::startSkillCooldown() {
    skillCooldownRemaining = getAllySkillConfig(type).cooldown;
}

int Ally::calculateSkillDamage(float multiplier) const {
    return std::max(0, static_cast<int>(
        getAttackPower() * multiplier + 0.5f));
}

int Ally::countEnemiesInRange(
    const GameContext& context, float range) const {
    int count = 0;
    const float rangeSquared = range * range;
    for (Entity* enemy : context.enemies) {
        if (!enemy || enemy->isDead() || enemy->getTeam() != Team::Enemy) {
            continue;
        }
        const sf::Vector2f offset = enemy->getPosition() - getPosition();
        if (offset.x * offset.x + offset.y * offset.y <= rangeSquared) {
            ++count;
        }
    }
    return count;
}

Entity* Ally::findClosestEnemyInRange(
    const GameContext& context, float range) const {
    Entity* closest = nullptr;
    float closestDistance = range * range;
    for (Entity* enemy : context.enemies) {
        if (!enemy || enemy->isDead() || enemy->getTeam() != Team::Enemy) {
            continue;
        }
        const sf::Vector2f offset = enemy->getPosition() - getPosition();
        const float distance = offset.x * offset.x + offset.y * offset.y;
        if (distance <= closestDistance) {
            closest = enemy;
            closestDistance = distance;
        }
    }
    return closest;
}

int Ally::damageEnemiesInRange(
    const GameContext& context, float range, float multiplier,
    bool strongEffect) {
    const float rangeSquared = range * range;
    const float damage = static_cast<float>(calculateSkillDamage(multiplier));
    int hitCount = 0;
    for (Entity* enemy : context.enemies) {
        if (!enemy || enemy->isDead() || enemy->getTeam() != Team::Enemy) {
            continue;
        }
        const sf::Vector2f offset = enemy->getPosition() - getPosition();
        if (offset.x * offset.x + offset.y * offset.y <= rangeSquared) {
            const float before = enemy->getHealth();
            enemy->takeDamage(damage);
            const float dealt = std::max(0.f, before - enemy->getHealth());
            if (dealt > 0.f) {
                ++hitCount;
                if (context.effects) {
                    context.effects->spawnImpact(
                        enemy->getPosition(), strongEffect
                            ? ImpactStyle::Strong : ImpactStyle::Magic);
                    context.effects->spawnFloatingNumber(
                        enemy->getPosition(), dealt, false, true);
                }
            }
        }
    }
    return hitCount;
}

bool Ally::castExplosiveArrow(GameContext& context) {
    const AllySkillConfig& config = getAllySkillConfig(type);
    Entity* enemy = findClosestEnemyInRange(context, config.triggerRange);
    if (!enemy) return false;

    sf::Vector2f direction = enemy->getPosition() - getPosition();
    const float length = std::sqrt(
        direction.x * direction.x + direction.y * direction.y);
    direction = length > 0.0001f
        ? direction / length
        : sf::Vector2f{1.f, 0.f};
    setAttackDirection(direction);

    auto projectile = std::make_unique<Projectiles>(
        getPosition(), direction, 500.f,
        static_cast<float>(calculateSkillDamage(config.primaryMultiplier)),
        getTeam(), ProjectileStyle::ExplosiveArrow);
    projectile->configureExplosion(
        static_cast<float>(calculateSkillDamage(config.secondaryMultiplier)),
        config.secondaryRadius);
    context.projectiles.push_back(std::move(projectile));

    startSkillCooldown();
    beginAttackCooldown();
    activateSkillVisual(32.f, sf::Color(255, 90, 30, 55));
    if (context.effects) {
        context.effects->spawnAllySkill(
            getPosition(), sf::Color(255, 190, 45), 48.f, config.name);
    }
    if (context.audioManager) context.audioManager->playSound("ally_skill");
    return true;
}

bool Ally::castArcaneBurst(GameContext& context) {
    const AllySkillConfig& config = getAllySkillConfig(type);
    if (countEnemiesInRange(context, config.triggerRange) <
        config.minimumEnemies) {
        return false;
    }
    damageEnemiesInRange(
        context, config.triggerRange, config.primaryMultiplier, false);
    startSkillCooldown();
    beginAttackCooldown();
    activateSkillVisual(
        config.triggerRange, sf::Color(165, 70, 235, 35));
    if (context.effects) {
        context.effects->spawnAllySkill(
            getPosition(), sf::Color(137, 45, 230),
            config.triggerRange, config.name);
    }
    if (context.audioManager) context.audioManager->playSound("ally_skill");
    return true;
}

bool Ally::castWhirlwind(GameContext& context) {
    const AllySkillConfig& config = getAllySkillConfig(type);
    if (countEnemiesInRange(context, config.triggerRange) <
        config.minimumEnemies) {
        return false;
    }
    const int hitCount = damageEnemiesInRange(
        context, config.triggerRange, config.primaryMultiplier, true);
    startSkillCooldown();
    beginAttackCooldown();
    activateSkillVisual(
        config.triggerRange, sf::Color(255, 165, 35, 35));
    if (context.effects) {
        context.effects->spawnAllySkill(
            getPosition(), sf::Color(245, 78, 30),
            config.triggerRange, config.name);
        if (hitCount > 0) context.effects->requestScreenShake(3.f);
    }
    if (context.audioManager) context.audioManager->playSound("ally_skill");
    return true;
}

void Ally::activateShieldWall(GameContext& context) {
    startSkillCooldown();
    activateSkillVisual(30.f, sf::Color(70, 155, 255, 70));
    if (context.effects) {
        context.effects->spawnAllySkill(
            getPosition(), sf::Color(70, 175, 255), 45.f,
            getAllySkillConfig(type).name);
    }
    if (context.audioManager) context.audioManager->playSound("ally_skill");
}

bool Ally::tryUseSkill(GameContext& context) {
    if (!context.allySkillsEnabled ||
        skillCooldownRemaining > 0.f || isDead()) {
        return false;
    }

    if (type == AllyType::Evangeline) {
        if (getHealth() <= getMaxHealth() * 0.35f) {
            activateShieldWall(context);
        }
        return false;
    }

    if (getIsAttacking() || coolDownTimer > 0.f || !currentWeapon) {
        return false;
    }

    switch (type) {
    case AllyType::Damian: return castExplosiveArrow(context);
    case AllyType::Junior: return castArcaneBurst(context);
    case AllyType::Lucas: return castWhirlwind(context);
    case AllyType::Evangeline: return false;
    }
    return false;
}

void Ally::forgetEntity(const Entity* entity) {
    Entity::forgetEntity(entity);
    if (target == entity) {
        target = nullptr;
        setIsAttacking(false);
    }
}

// ===============================
// TARGET MANAGEMENT
// ===============================

bool Ally::isInRange(const Entity* target) const {
    if (!target) return false;
    float dx = target->getX() - getX();
    float dy = target->getY() - getY();
    const float range = getAttackRange();
    return range > 0.f &&
           (dx * dx + dy * dy) <= (range * range);
}

float Ally::getAttackRange() const {
    return currentWeapon ? currentWeapon->getAttackRange() : 0.f;
}

//logic tim kiem va chon quai gan nhat lam muc tieu va danh den khi quai do chet 

void Ally::updateTarget(const GameContext& context) {
    Entity* bestTarget = nullptr;
    const float range = getAttackRange();
    float minDistSq = range * range;

    for (auto* entity : context.enemies) {
        if (entity == nullptr || entity->isDead()) continue;
        float dx = entity->getX() - getX();
        float dy = entity->getY() - getY();
        float distSq = dx * dx + dy * dy;
        if (distSq <= minDistSq) {
            minDistSq = distSq;
            bestTarget = entity;
        }
    }

    if (target != bestTarget && isAttacking) {
        setIsAttacking(false);
    }
    target = bestTarget;
}
sf::FloatRect Ally::getCollisionBox() const {
    constexpr sf::Vector2f gameplaySize{40.f, 40.f};
    return {getPosition() - gameplaySize / 2.f, gameplaySize};
}

sf::FloatRect Ally::getHurtBox() const {
    constexpr sf::Vector2f gameplaySize{40.f, 40.f};
    return {getPosition() - gameplaySize / 2.f, gameplaySize};
}

std::unique_ptr<Ally> createAlly(
    AllyType type,
    sf::Vector2f position,
    TextureManager& textureManager
) {
    return std::make_unique<Ally>(
        position.x, position.y, textureManager, type);
}
