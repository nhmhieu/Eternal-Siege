#include "RadiantPulse.h"

#include "Ally.h"
#include "Player.h"

#include <algorithm>
#include <cmath>

RadiantPulseResult RadiantPulse::tryActivate(
    Player& player, const std::vector<Ally*>& allies,
    bool combatActive) {
    RadiantPulseResult result;
    if (!combatActive || !isReady() || player.isDead()) return result;

    const float radiusSquared = RADIUS * RADIUS;
    for (Ally* ally : allies) {
        if (!ally || ally->isDead() ||
            ally->getHealth() >= ally->getMaxHealth()) {
            continue;
        }
        const sf::Vector2f offset = ally->getPosition() - player.getPosition();
        if (offset.x * offset.x + offset.y * offset.y > radiusSquared) {
            continue;
        }

        const float requested = std::round(
            ally->getMaxHealth() * HEAL_RATIO);
        const float restored = ally->heal(requested);
        if (restored > 0.f) {
            result.heals.push_back({ally, restored});
        }
    }

    if (!result.heals.empty()) {
        result.activated = true;
        cooldownRemaining = COOLDOWN;
    }
    return result;
}

void RadiantPulse::update(float deltaTime, bool combatActive, bool paused) {
    if (paused || !combatActive || cooldownRemaining <= 0.f) return;
    cooldownRemaining = std::max(0.f, cooldownRemaining - deltaTime);
}

void RadiantPulse::reset() {
    cooldownRemaining = 0.f;
}
