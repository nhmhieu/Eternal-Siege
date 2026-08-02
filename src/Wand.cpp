#include "Wand.h"

#include "Entity.h"
#include "GameContext.h"
#include "MagicBolt.h"
#include "Effects.h"

#include <memory>

sf::FloatRect Wand::getHitbox(
    sf::Vector2f entityCenter, sf::Vector2f) {
    return {entityCenter - sf::Vector2f{15.f, 15.f}, {30.f, 30.f}};
}

bool Wand::isHitting(sf::Vector2f attackerPosition,
                     sf::Vector2f,
                     sf::Vector2f targetPosition) {
    const sf::Vector2f offset = targetPosition - attackerPosition;
    return offset.x * offset.x + offset.y * offset.y <=
           attackRange * attackRange;
}

void Wand::triggerAction(Entity* attacker,
                         GameContext& context,
                         CombatManager&) {
    if (!attacker || attacker->isDead() ||
        !attacker->getIsAttacking() || hasAttacked) {
        return;
    }
    context.projectiles.push_back(std::make_unique<MagicBolt>(
        attacker->getPosition(),
        attacker->getAttackDirection(),
        projectileSpeed,
        attacker->getAttackPower(),
        attacker->getTeam()));
    if (context.effects) {
        context.effects->spawnShot(
            attacker->getPosition(), attacker->getAttackDirection(), true);
    }
    hasAttacked = true;
}
