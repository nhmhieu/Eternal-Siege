#include "Monster.h"
#include "CombatManager.h"
#include "GameContext.h"
#include "Map.h"
#include "Weapon.h"
#include "Effects.h"

#include <cmath>
#include <cstdint>
#include <limits>

namespace {
float lengthSquared(sf::Vector2f value) {
    return value.x * value.x + value.y * value.y;
}

sf::Vector2f normalized(sf::Vector2f value) {
    const float length = std::sqrt(lengthSquared(value));
    return length > 0.0001f ? value / length : sf::Vector2f{};
}
}

Monster::Monster(float x, float y, float hp, float maximumHealth,
                 float range, float cooldown, float speed, float damage)
    : Entity(x, y, hp, maximumHealth),
      attackRange(range), moveSpeed(speed) {
    team = Team::Enemy;
    attackPower = damage;
    attackCoolDown = cooldown;
    coolDownTimer = 0.f;
    attackDuration = 0.22f;

    monsterShape.setSize({32.f, 32.f});
    monsterShape.setOrigin({16.f, 16.f});
    monsterShape.setFillColor(sf::Color(205, 65, 65));
    monsterShape.setPosition(position);
    previousPosition = position;
}

void Monster::updateTarget(const std::vector<Entity*>& targets) {
    Entity* closest = nullptr;
    float bestDistance = std::numeric_limits<float>::max();
    for (Entity* candidate : targets) {
        if (!candidate || candidate->isDead()) continue;
        const float distance = lengthSquared(candidate->getPosition() - position);
        if (distance < bestDistance) {
            bestDistance = distance;
            closest = candidate;
        }
    }
    currentTarget = closest;
}

void Monster::forgetEntity(const Entity* entity) {
    Entity::forgetEntity(entity);
    if (currentTarget == entity) {
        currentTarget = nullptr;
        path.clear();
        waypointIndex = 0;
        setIsAttacking(false);
    }
}

void Monster::setPosition(float x, float y) {
    Entity::setPosition(x, y);
    monsterShape.setPosition(position);
}

void Monster::moveWithCollision(
    sf::Vector2f displacement,
    const Map& map
) {
    const sf::Vector2f navigationHalfExtents =
        getCollisionBox().size / 2.f;
    const sf::Vector2f resolved = map.resolveMovement(
        position, navigationHalfExtents, displacement);
    setPosition(resolved.x, resolved.y);
}

void Monster::rebuildPath(const Map& map) {
    if (!currentTarget) {
        path.clear();
        waypointIndex = 0;
        return;
    }

    const auto start = map.worldToGrid(position);
    const auto goal = map.worldToGrid(currentTarget->getPosition());

    path = map.findPathBFS(start, goal);
    waypointIndex = path.size() > 1 ? 1 : 0;
    lastGoalCell = goal;
}

void Monster::followPath(float dt, const Map& map) {
    if (!currentTarget) return;

    const sf::Vector2f toTarget = currentTarget->getPosition() - position;
    gap = std::sqrt(lengthSquared(toTarget));
    attackDirection = normalized(toTarget);
    if (gap <= attackRange) return;

    if (path.empty() || waypointIndex >= path.size()) rebuildPath(map);
    if (path.empty()) return;

    sf::Vector2f waypoint = map.gridToWorld(path[waypointIndex]);
    sf::Vector2f toWaypoint = waypoint - position;
    float waypointDistance = std::sqrt(lengthSquared(toWaypoint));
    if (waypointDistance <= 4.f) {
        ++waypointIndex;
        if (waypointIndex >= path.size()) return;
        waypoint = map.gridToWorld(path[waypointIndex]);
        toWaypoint = waypoint - position;
        waypointDistance = std::sqrt(lengthSquared(toWaypoint));
    }

    const sf::Vector2f moveDirection = normalized(toWaypoint);
    const float step = std::min(moveSpeed * dt, waypointDistance);
    const sf::Vector2f before = position;
    moveWithCollision(moveDirection * step, map);
    if (position == before && step > 0.f) {
        path.clear();
    }
}

void Monster::update(GameContext& context) {
    if (context.paused) return;
    Entity::update(context);
    if (isDead()) {
        currentTarget = nullptr;
        path.clear();
        waypointIndex = 0;
        if (!isDying) startDying();
        updateDeadTimer(context);
        updatePresentation(context.effects);
        return;
    }

    // Luôn chọn lại từ danh sách sống: không giữ con trỏ tới Ally đã bị xóa.
    updateTarget(context.players);
    if (!currentTarget || !context.map) return;

    pathRefreshTimer -= context.deltaTime;
    const auto goalCell = context.map->worldToGrid(currentTarget->getPosition());
    if (pathRefreshTimer <= 0.f || goalCell != lastGoalCell) {
        pathRefreshTimer = 0.35f;
        path.clear();
        rebuildPath(*context.map);
    }
    followPath(context.deltaTime, *context.map);

    if (gap <= attackRange && coolDownTimer <= 0.f && !isAttacking) {
        startAttacking();
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

void Monster::draw(sf::RenderWindow& window) {
    window.draw(monsterShape);
}

void Monster::drawShadow(sf::RenderWindow& window) const {
    const float shadowRadius = std::max(10.f, visualSize.x * 0.28f);
    sf::CircleShape shadow(shadowRadius);
    shadow.setOrigin({shadowRadius, shadowRadius});
    shadow.setScale({1.35f, 0.38f});
    shadow.setPosition({position.x, position.y + collisionSize.y * 0.45f});
    shadow.setFillColor(sf::Color(8, 5, 15, isDying ? 35 : 100));
    window.draw(shadow);
}

void Monster::setPresentationTexture(
    const sf::Texture* texture, sf::IntRect visibleBounds,
    float desiredHeight, sf::Color tint) {
    if (!texture) return;
    monsterTexture = texture;
    presentationTint = tint;
    monsterShape.setTexture(texture, true);
    if (visibleBounds.size.x > 0 && visibleBounds.size.y > 0) {
        monsterShape.setTextureRect(visibleBounds);
        const float aspect = static_cast<float>(visibleBounds.size.x) /
                             static_cast<float>(visibleBounds.size.y);
        const float minWidth = desiredHeight * 0.62f;
        const float maxWidth = desiredHeight * 1.16f;
        visualSize = {
            std::clamp(desiredHeight * aspect, minWidth, maxWidth),
            desiredHeight
        };
        monsterShape.setSize(visualSize);
        monsterShape.setOrigin(visualSize / 2.f);
    }
    monsterShape.setFillColor(tint);
}

void Monster::updatePresentation(Effects* effects) {
    const sf::Vector2f moved = position - previousPosition;
    visuallyMoving = lengthSquared(moved) > 0.001f;
    previousPosition = position;
    if (visuallyMoving && !isDying) {
        footstepDistance += std::sqrt(lengthSquared(moved));
        const float spacing = isBoss() ? 40.f : (isElite() ? 29.f : 20.f);
        if (footstepDistance >= spacing) {
            footstepDistance = std::fmod(footstepDistance, spacing);
            if (effects) {
                const FootstepStyle style = isBoss()
                    ? FootstepStyle::Boss
                    : (isElite() ? FootstepStyle::Elite
                                 : FootstepStyle::Normal);
                effects->spawnFootstep(
                    {position.x, position.y + collisionSize.y * 0.42f}, style);
            }
        }
    }

    const float cadence = eliteVisual ? 8.f : 11.f;
    const float bob = visuallyMoving
        ? std::abs(std::sin(visualTime * cadence)) * 3.2f
        : std::sin(visualTime * 3.4f) * 0.8f;
    float rotation = visuallyMoving
        ? std::sin(visualTime * cadence) * 2.4f
        : std::sin(visualTime * 2.2f) * 0.7f;
    float stretchX = 1.f;
    float stretchY = 1.f;
    sf::Vector2f visualPosition = position - sf::Vector2f(0.f, bob);

    if (isAttacking) {
        const float pulse = std::sin(
            getAttackAnimationProgress() * 3.14159265358979323846f);
        visualPosition += attackDirection * (eliteVisual ? 7.f : 5.f) * pulse;
        stretchX += 0.08f * pulse;
        stretchY -= 0.07f * pulse;
        rotation += attackDirection.x * 7.f * pulse;
    }

    const float facing = attackDirection.x < -0.05f ? -1.f : 1.f;
    sf::Color tint = presentationTint;
    if (isDying) {
        const float progress = std::clamp(
            deadTimer / std::max(0.01f, deadAnimationDuration), 0.f, 1.f);
        rotation += facing * 78.f * progress;
        stretchX *= 1.f - progress * 0.28f;
        stretchY *= 1.f - progress * 0.45f;
        tint.a = static_cast<std::uint8_t>(255.f * (1.f - progress));
    } else if (healingFlashTimer > 0.f) {
        tint = sf::Color(135, 255, 195);
    } else if (hurtFlashTimer > 0.f) {
        tint = sf::Color(255, 125, 125);
    }

    monsterShape.setPosition(visualPosition);
    monsterShape.setScale({facing * stretchX, stretchY});
    monsterShape.setRotation(sf::degrees(rotation));
    monsterShape.setFillColor(tint);
}

sf::FloatRect Monster::getCollisionBox() const {
    return {position - collisionSize / 2.f, collisionSize};
}

sf::FloatRect Monster::getHurtBox() const {
    return {position - hurtBoxSize / 2.f, hurtBoxSize};
}

int Monster::claimGoldReward() {
    if (rewardClaimed || !isDead()) return 0;
    rewardClaimed = true;
    return goldReward;
}
