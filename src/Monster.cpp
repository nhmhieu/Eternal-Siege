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

    constexpr float WAYPOINT_TOLERANCE = 14.f;
    sf::Vector2f waypoint = map.gridToWorld(path[waypointIndex]);
    sf::Vector2f toWaypoint = waypoint - position;
    float waypointDistance = std::sqrt(lengthSquared(toWaypoint));

    while (waypointDistance <= WAYPOINT_TOLERANCE && waypointIndex < path.size()) {
        ++waypointIndex;
        if (waypointIndex >= path.size()) break;
        waypoint = map.gridToWorld(path[waypointIndex]);
        toWaypoint = waypoint - position;
        waypointDistance = std::sqrt(lengthSquared(toWaypoint));
    }

    sf::Vector2f moveDirection;
    if (waypointIndex < path.size()) {
        moveDirection = normalized(toWaypoint);
    } else {
        moveDirection = normalized(toTarget);
    }

    const float step = moveSpeed * dt;
    if (step <= 0.f) return;

    const sf::Vector2f before = position;
    moveWithCollision(moveDirection * step, map);
    if (position == before) {
        path.clear();
        waypointIndex = 0;
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
    if (path.empty() || waypointIndex >= path.size() || goalCell != lastGoalCell || pathRefreshTimer <= 0.f) {
        pathRefreshTimer = 0.30f;
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
    if (isElite()) {
        const float auraRadius = visualSize.x * 0.45f;
        sf::CircleShape auraRing(auraRadius);
        auraRing.setOrigin({auraRadius, auraRadius});
        auraRing.setScale({1.4f, 0.42f});
        auraRing.setPosition({position.x, position.y + collisionSize.y * 0.42f});
        auraRing.setFillColor(sf::Color(255, 160, 40, 50));
        auraRing.setOutlineColor(sf::Color(255, 200, 80, 140));
        auraRing.setOutlineThickness(1.5f);
        window.draw(auraRing);
    }
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

    const sf::Vector2u texSize = texture->getSize();
    if (texSize.x == 1024 && texSize.y == 1536) {
        isSpriteSheet = true;
        frameSize = {256, 384};
        const float aspect = static_cast<float>(frameSize.x) / static_cast<float>(frameSize.y);
        visualSize = {desiredHeight * aspect, desiredHeight};
        monsterShape.setSize(visualSize);
        monsterShape.setOrigin(visualSize / 2.f);
        monsterShape.setTextureRect({{0, 0}, frameSize});
    } else if (visibleBounds.size.x > 0 && visibleBounds.size.y > 0) {
        isSpriteSheet = false;
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
    const float movedDistance = std::hypot(moved.x, moved.y);
    visuallyMoving = movedDistance > 0.001f;
    previousPosition = position;

    const float strideDistance = isBoss() ? 24.f : (isElite() ? 18.f : 14.f);

    if (visuallyMoving && !isDying) {
        walkDistanceAccumulator += movedDistance;
        footstepDistance += movedDistance;

        if (footstepDistance >= strideDistance * 2.f) {
            footstepDistance = std::fmod(footstepDistance, strideDistance * 2.f);
            if (effects) {
                const FootstepStyle style = isBoss()
                    ? FootstepStyle::Boss
                    : (isElite() ? FootstepStyle::Elite
                                 : FootstepStyle::Normal);
                effects->spawnFootstep(
                    {position.x, position.y + collisionSize.y * 0.42f}, style);
            }
        }
    } else {
        walkDistanceAccumulator = 0.f;
    }

    // Direction selection from actual movement displacement
    if (visuallyMoving) {
        if (std::abs(moved.x) > std::abs(moved.y)) {
            animDirectionRow = moved.x < 0.f ? 1 : 2; // 1: Left, 2: Right
        } else {
            animDirectionRow = moved.y < 0.f ? 3 : 0; // 3: Up, 0: Down
        }
    } else if (std::abs(attackDirection.x) > 0.05f || std::abs(attackDirection.y) > 0.05f) {
        if (std::abs(attackDirection.x) > std::abs(attackDirection.y)) {
            animDirectionRow = attackDirection.x < 0.f ? 1 : 2;
        } else {
            animDirectionRow = attackDirection.y < 0.f ? 3 : 0;
        }
    }

    if (isSpriteSheet) {
        if (visuallyMoving && !isDying) {
            const int totalSteps = static_cast<int>(walkDistanceAccumulator / strideDistance);
            animFrameIndex = totalSteps % 4; // Cycles through walk frames 0, 1, 2, 3
        } else {
            animFrameIndex = 0; // Clean neutral idle stance
        }

        const int frameX = animFrameIndex * frameSize.x;
        const int frameY = animDirectionRow * frameSize.y;
        monsterShape.setTextureRect({{frameX, frameY}, frameSize});
    }

    float rotation = 0.f;
    sf::Vector2f visualPosition = position;

    if (isAttacking) {
        const float pulse = std::sin(
            getAttackAnimationProgress() * 3.14159265358979323846f);
        visualPosition += attackDirection * (isBoss() ? 9.f : (isElite() ? 7.f : 5.f)) * pulse;
        rotation += attackDirection.x * 6.f * pulse;
    }

    sf::Color tint = presentationTint;
    if (isDying) {
        const float progress = std::clamp(
            deadTimer / std::max(0.01f, deadAnimationDuration), 0.f, 1.f);
        rotation += (animDirectionRow == 1 ? -1.f : 1.f) * 78.f * progress;
        tint.a = static_cast<std::uint8_t>(255.f * (1.f - progress));
    } else if (healingFlashTimer > 0.f) {
        tint = sf::Color(135, 255, 195);
    } else if (hurtFlashTimer > 0.f) {
        tint = sf::Color(255, 125, 125);
    }

    monsterShape.setPosition(visualPosition);
    monsterShape.setScale({1.f, 1.f});
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
