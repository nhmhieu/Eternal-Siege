#include "CombatManager.h"
#include <iostream>

void CombatManager::processAttack(Entity* attacker, Weapon* weapon, std::vector<Entity*>& targets) {
    if (!attacker || !weapon || !attacker->getIsAttacking()) return;

    sf::FloatRect attackArea = weapon->getHitbox(
        { attacker->getX(), attacker->getY() },
        attacker->getAttackDirection()
    );

    for (auto* target : targets) {
        if (!target || target->isDead()) continue;
        if (target->getTeam() == attacker->getTeam()) continue;

        if (!attackArea.findIntersection(target->getHurtBox()).has_value()) continue;

        if (!weapon->isHitting({ attacker->getX(), attacker->getY() },
            attacker->getAttackDirection(),
            { target->getX(), target->getY() })) continue;

        if (!weapon->isHit(target)) {
            target->takeDamage(attacker->getAttackPower());
            weapon->addHit(target);
            std::cout << "Da danh trung quai!" << std::endl;
        }
    }
}

void CombatManager::resetAttack() {
    hitEntities.clear();
}

bool CombatManager::hasHit(Entity* target) const {
    return hitEntities.find(target) != hitEntities.end();
}