#include "EntityCollision.h"

#include "Ally.h"
#include "Map.h"
#include "Monster.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr float EPSILON = 0.001f;
constexpr float MAX_SEPARATION_SPEED = 28.f;

float radiusOf(const Entity& entity) {
    const sf::Vector2f size = entity.getCollisionBox().size;
    return std::max(1.f, std::min(size.x, size.y) * 0.5f);
}

float weightOf(const Entity& entity) {
    if (dynamic_cast<const Ally*>(&entity)) return 10.0f;
    if (const auto* m = dynamic_cast<const Monster*>(&entity)) {
        if (m->isBoss()) return 3.0f;
        if (m->isElite()) return 1.5f;
        return 1.0f;
    }
    return 1.0f;
}
}

namespace EntityCollision {

void separateLivingEntities(
    const Map& map,
    const std::vector<Monster*>& monsters,
    const std::vector<Ally*>& allies,
    float dt
) {
    if (monsters.empty() || dt <= 0.f) return;

    std::vector<Monster*> activeMonsters;
    activeMonsters.reserve(monsters.size());
    for (Monster* m : monsters) {
        if (m && !m->isDead()) {
            activeMonsters.push_back(m);
        }
    }
    if (activeMonsters.empty()) return;

    std::vector<Ally*> activeAllies;
    activeAllies.reserve(allies.size());
    for (Ally* a : allies) {
        if (a && !a->isDead()) {
            activeAllies.push_back(a);
        }
    }

    const std::size_t count = activeMonsters.size();
    std::vector<sf::Vector2f> separationNudges(count, sf::Vector2f{0.f, 0.f});

    for (std::size_t i = 0; i < count; ++i) {
        Monster* a = activeMonsters[i];
        const sf::Vector2f posA = a->getPosition();
        const float radiusA = radiusOf(*a);
        const float weightA = weightOf(*a);

        for (std::size_t j = i + 1; j < count; ++j) {
            Monster* b = activeMonsters[j];
            const sf::Vector2f posB = b->getPosition();
            const float radiusB = radiusOf(*b);
            const float weightB = weightOf(*b);

            const sf::Vector2f delta = posA - posB;
            float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);
            const float combinedRadius = radiusA + radiusB;

            if (dist < combinedRadius) {
                sf::Vector2f dir;
                if (dist > EPSILON) {
                    dir = delta / dist;
                } else {
                    const float angle = static_cast<float>((i + j) % 8) * 0.785398f;
                    dir = {std::cos(angle), std::sin(angle)};
                    dist = EPSILON;
                }

                const float overlap = combinedRadius - dist;
                const float totalWeight = weightA + weightB;
                const float ratioA = weightB / totalWeight;
                const float ratioB = weightA / totalWeight;

                separationNudges[i] += dir * (overlap * ratioA);
                separationNudges[j] -= dir * (ratioB * overlap);
            }
        }

        for (std::size_t k = 0; k < activeAllies.size(); ++k) {
            Ally* ally = activeAllies[k];
            const sf::Vector2f posAlly = ally->getPosition();
            const float radiusAlly = radiusOf(*ally);

            const sf::Vector2f delta = posA - posAlly;
            float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);
            const float combinedRadius = radiusA + radiusAlly;

            if (dist < combinedRadius) {
                sf::Vector2f dir;
                if (dist > EPSILON) {
                    dir = delta / dist;
                } else {
                    dir = {1.f, 0.f};
                    dist = EPSILON;
                }
                const float overlap = combinedRadius - dist;
                separationNudges[i] += dir * overlap;
            }
        }
    }

    const float maxStep = MAX_SEPARATION_SPEED * dt;
    for (std::size_t i = 0; i < count; ++i) {
        Monster* monster = activeMonsters[i];
        const sf::Vector2f nudge = separationNudges[i];
        const float nudgeLen = std::sqrt(nudge.x * nudge.x + nudge.y * nudge.y);
        if (nudgeLen > EPSILON) {
            sf::Vector2f displacement = nudge * 0.35f;
            const float dispLen = std::sqrt(displacement.x * displacement.x + displacement.y * displacement.y);
            if (dispLen > maxStep) {
                displacement = (displacement / dispLen) * maxStep;
            }

            const sf::Vector2f halfExtents = monster->getCollisionBox().size / 2.f;
            const sf::Vector2f resolved = map.resolveMovement(
                monster->getPosition(), halfExtents, displacement);
            monster->setPosition(resolved.x, resolved.y);
        }
    }
}

}
