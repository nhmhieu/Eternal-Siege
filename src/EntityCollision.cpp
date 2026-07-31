#include "EntityCollision.h"

#include "Ally.h"
#include "Map.h"
#include "Monster.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr float EPSILON = 0.01f;
constexpr int SEPARATION_ITERATIONS = 4;

float radiusOf(const Entity& entity) {
    const sf::Vector2f size = entity.getCollisionBox().size;
    return std::max(1.f, std::min(size.x, size.y) * 0.5f);
}

bool canPlace(const Map& map, const Monster& monster, sf::Vector2f center) {
    const sf::Vector2f size = monster.getCollisionBox().size;
    return !map.collidesWithSolid({center - size / 2.f, size});
}

bool placeMonster(
    const Map& map,
    Monster& monster,
    sf::Vector2f desiredCorrection
) {
    const sf::Vector2f current = monster.getPosition();
    const sf::Vector2f candidates[] = {
        current + desiredCorrection,
        current + sf::Vector2f(desiredCorrection.x, 0.f),
        current + sf::Vector2f(0.f, desiredCorrection.y),
        current + sf::Vector2f(-desiredCorrection.y, desiredCorrection.x),
        current + sf::Vector2f(desiredCorrection.y, -desiredCorrection.x),
        current - desiredCorrection
    };
    for (const sf::Vector2f candidate : candidates) {
        const sf::Vector2f applied = candidate - current;
        if (std::abs(applied.x) <= EPSILON &&
            std::abs(applied.y) <= EPSILON) {
            continue;
        }
        if (canPlace(map, monster, candidate)) {
            monster.setPosition(candidate.x, candidate.y);
            return true;
        }
    }
    return false;
}

sf::Vector2f separationDirection(
    sf::Vector2f delta,
    std::size_t firstIndex,
    std::size_t secondIndex
) {
    const float distanceSquared =
        delta.x * delta.x + delta.y * delta.y;
    if (distanceSquared > EPSILON * EPSILON) {
        return delta / std::sqrt(distanceSquared);
    }

    // Stable direction for coincident centers: no random loop, division by
    // zero or NaN. Pair order makes the choice deterministic.
    return ((firstIndex + secondIndex) % 2 == 0)
        ? sf::Vector2f{1.f, 0.f}
        : sf::Vector2f{0.f, 1.f};
}
}

namespace EntityCollision {

void separateLivingEntities(
    const Map& map,
    const std::vector<Monster*>& monsters,
    const std::vector<Ally*>& allies
) {
    for (int iteration = 0;
         iteration < SEPARATION_ITERATIONS;
         ++iteration) {
        for (std::size_t first = 0; first < monsters.size(); ++first) {
            Monster* a = monsters[first];
            if (!a || a->isDead()) continue;

            for (std::size_t second = first + 1;
                 second < monsters.size();
                 ++second) {
                Monster* b = monsters[second];
                if (!b || b->isDead()) continue;

                const sf::Vector2f delta =
                    b->getPosition() - a->getPosition();
                const sf::Vector2f direction =
                    separationDirection(delta, first, second);
                const float distance = std::sqrt(
                    delta.x * delta.x + delta.y * delta.y);
                const float overlap =
                    radiusOf(*a) + radiusOf(*b) - distance;
                if (overlap <= EPSILON) continue;

                const sf::Vector2f halfCorrection =
                    direction * ((overlap + EPSILON) * 0.5f);
                const bool movedA =
                    placeMonster(map, *a, -halfCorrection);
                const bool movedB =
                    placeMonster(map, *b, halfCorrection);

                if (!movedA && movedB) {
                    placeMonster(map, *b, halfCorrection);
                } else if (movedA && !movedB) {
                    placeMonster(map, *a, -halfCorrection);
                }
            }
        }

        // Allies are fixed formation anchors; only the Monster is corrected.
        for (std::size_t monsterIndex = 0;
             monsterIndex < monsters.size();
             ++monsterIndex) {
            Monster* monster = monsters[monsterIndex];
            if (!monster || monster->isDead()) continue;

            for (std::size_t allyIndex = 0;
                 allyIndex < allies.size();
                 ++allyIndex) {
                Ally* ally = allies[allyIndex];
                if (!ally || ally->isDead()) continue;

                const sf::Vector2f delta =
                    monster->getPosition() - ally->getPosition();
                const sf::Vector2f direction = separationDirection(
                    delta, monsterIndex, allyIndex);
                const float distance = std::sqrt(
                    delta.x * delta.x + delta.y * delta.y);
                const float overlap =
                    radiusOf(*monster) + radiusOf(*ally) - distance;
                if (overlap <= EPSILON) continue;

                placeMonster(
                    map,
                    *monster,
                    direction * (overlap + EPSILON)
                );
            }
        }
    }
}

}
