#include "CombatManager.h"
#include "GameContext.h" 
#include "Projectiles.h"
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
            // std :: cout << "Current health : " << target->getHealth() << std :: endl ; 
            // std::cout << "Da danh trung quai!" << std::endl;
        }
    }
}

void CombatManager::resetAttack() {
    hitEntities.clear();
}

bool CombatManager::hasHit(Entity* target) const {
    return hitEntities.find(target) != hitEntities.end();
}

//Ham xu li logic rieng danh cho dan bay 
void CombatManager::processProjectiles(const GameContext& context, const std::vector<Entity*>& targets) {
    // Duyệt qua tất cả các mũi tên đang bay trên bản đồ
    for (auto& proj : context.projectiles) {
        if (!proj->isActive()) continue;

        // 1. Cập nhật vị trí bay của mũi tên
        proj->update(context);

        // 2. Kiểm tra va chạm với từng quái
        for (auto* target : targets) {
            if (!target || target->isDead()) continue;
            if (target->getTeam() == proj->getShooterTeam()) continue; // Không bắn đồng đội

            // Gọi hàm check va chạm của chính mũi tên đó
            if (proj->isHitting(target)) {
                target->takeDamage(proj->getDamage()); // Trừ máu quái
                proj->deactivate();                    // Hủy mũi tên ngay lập tức sau khi trúng
                break; 
            }
        }
    }
}