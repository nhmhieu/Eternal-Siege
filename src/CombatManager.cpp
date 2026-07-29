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
            target->takeDamage(static_cast<float>(weapon->getDamage()));
            weapon->addHit(target);
            std :: cout << "Current health : " << target->getHealth() << std :: endl ; 
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

//Ham xu li logic rieng danh cho dan bay 
void CombatManager::processProjectiles(GameContext& context, const std::vector<Entity*>& targets) {
    auto it = context.projectiles.begin();
    
    while (it != context.projectiles.end()) {
        auto* proj = *it; 

        // --- LỚP BẢO VỆ CHỐNG CRASH (BẮT BUỘC PHẢI CÓ) ---
        if (proj == nullptr) {
            it = context.projectiles.erase(it); 
            continue;
        }

        // Nếu đạn đã inactive từ trước, xóa luôn khỏi danh sách và bộ nhớ
        if (!proj->isActive()) {
            delete proj;
            it = context.projectiles.erase(it);
            continue;
        }

        // 1. Cập nhật vị trí bay của mũi tên
        proj->update(context); 

        // 2. Kiểm tra va chạm với từng quái
        for (auto* target : targets) {
            if (!target || target->isDead()) continue;
            if (target->getTeam() == proj->getShooterTeam()) continue; // Bỏ qua cùng team

            if (proj->isHitting(target)) {
                target->takeDamage(proj->getDamage());
                
                // Hạn chế std::cout nếu không thực sự cần thiết để tránh lag
                // std::cout << "Da ban trung quai, mau quai hien tai : " << target->getHealth() << std::endl; 
                
                proj->deactivate(); // Đánh dấu đạn đã trúng mục tiêu
                break; // Trúng 1 mục tiêu là dừng vòng lặp check quái
            }
        }

        // 3. Kiểm tra lại lần nữa sau khi update/va chạm: nếu đạn đã chết (deactivate) thì xóa
        if (!proj->isActive()) {
            delete proj; // Giải phóng bộ nhớ tránh memory leak
            it = context.projectiles.erase(it); // Xóa khỏi vector và nhận iterator mới an toàn
        } else {
            ++it; // Chỉ tăng iterator khi không có phần tử nào bị xóa
        }
    }

}
