#include "Monster.h"
#include "CombatManager.h"
#include "GameContext.h"
#include "Map.h"
#include "Weapon.h"

#include <cmath>
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
    Entity::update(context);
    if (isDead()) {
        if (!isDying) startDying();
        updateDeadTimer(context);
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
    monsterShape.setPosition(position);
}

void Monster::draw(sf::RenderWindow& window) {
    window.draw(monsterShape);
}

sf::FloatRect Monster::getCollisionBox() const {
    sf::Vector2f collisionSize = monsterShape.getGlobalBounds().size;
    // Keep the Boss able to pass through the map's 48 px gates while using
    // the same centered collision rule as every other Monster subtype.
    collisionSize.x = std::min(collisionSize.x, 44.f);
    collisionSize.y = std::min(collisionSize.y, 44.f);
    return {position - collisionSize / 2.f, collisionSize};
}

sf::FloatRect Monster::getHurtBox() const {
    return monsterShape.getGlobalBounds();
}

int Monster::claimGoldReward() {
    if (rewardClaimed || !isDead()) return 0;
    rewardClaimed = true;
    return goldReward;
}
