#include "Bow.h"
#include "Arrow.h"
#include "CombatManager.h"
#include "GameContext.h"
#include "Effects.h"
#include "AudioManager.h"
#include <memory>

sf::FloatRect Bow::getHitbox(sf::Vector2f entityCenter, sf::Vector2f) {
    // Tạm thời: hitbox nhỏ xung quanh entity
    return sf::FloatRect(entityCenter - sf::Vector2f(15.f, 15.f), sf::Vector2f(30.f, 30.f));
}

bool Bow::isHitting(sf::Vector2f attackerPos, sf::Vector2f, sf::Vector2f targetPos) {
    // Kiểm tra khoảng cách đơn giản
    float dx = targetPos.x - attackerPos.x;
    float dy = targetPos.y - attackerPos.y;
    return (dx * dx + dy * dy) <= attackRange * attackRange;
}

void Bow::fire(
    sf::Vector2f position,
    sf::Vector2f direction,
    Team shooterTeam,
    float damage,
    GameContext& context
) {
    context.projectiles.push_back(
        std::make_unique<Arrow>(position, direction, projectTileSpeed, damage, shooterTeam));
}

void Bow::triggerAction(Entity* attacker, GameContext& context, CombatManager&) {
    if (attacker && !attacker->isDead() &&
        attacker->getIsAttacking() && !hasAttacked) {
        this->fire(attacker->getPosition(), attacker->getAttackDirection(),
            attacker->getTeam(), attacker->getAttackPower(), context);
        if (context.effects) {
            context.effects->spawnShot(
                attacker->getPosition(), attacker->getAttackDirection(), false);
        }
        if (context.audioManager) {
            context.audioManager->playSound("bow_shot");
        }
        hasAttacked = true;
    }
}
