#include "CombatManager.h"
#include "GameContext.h" 
#include "Projectiles.h"
#include "Effects.h"
#include "Ally.h"
#include "Monster.h"

#include <algorithm>

void CombatManager::processAttack(Entity* attacker, Weapon* weapon,
                                  std::vector<Entity*>& targets,
                                  Effects* effects) {
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
            const float healthBefore = target->getHealth();
            target->takeDamage(attacker->getAttackPower());
            const float actualDamage = std::max(
                0.f, healthBefore - target->getHealth());
            if (effects) {
                ImpactStyle style = attacker->getTeam() == Team::Enemy
                    ? ImpactStyle::Enemy : ImpactStyle::Physical;
                if (const auto* ally = dynamic_cast<const Ally*>(attacker);
                    ally && ally->getType() == AllyType::Lucas) {
                    style = ImpactStyle::Strong;
                }
                if (const auto* monster = dynamic_cast<const Monster*>(attacker);
                    monster && (monster->isElite() || monster->isBoss())) {
                    style = ImpactStyle::Strong;
                    if (monster->isBoss()) effects->requestScreenShake(2.5f);
                }
                effects->spawnImpact(
                    target->getPosition(), style);
                if (actualDamage > 0.f) {
                    effects->spawnFloatingNumber(
                        target->getPosition(), actualDamage, false, false);
                }
            }
            weapon->addHit(target);
            hitEntities.insert(target);
        }
    }

}

void CombatManager::resetAttack() {
    hitEntities.clear();
}

bool CombatManager::hasHit(Entity* target) const {
    return hitEntities.find(target) != hitEntities.end();
}

void CombatManager::forgetEntity(const Entity* entity) {
    hitEntities.erase(const_cast<Entity*>(entity));
}

//Ham xu li logic rieng danh cho dan bay 
void CombatManager::processProjectiles(GameContext& context, const std::vector<Entity*>& targets) {
    auto it = context.projectiles.begin();
    
    while (it != context.projectiles.end()) {
        Projectiles* proj = it->get();

        // --- LỚP BẢO VỆ CHỐNG CRASH (BẮT BUỘC PHẢI CÓ) ---
        if (proj == nullptr) {
            it = context.projectiles.erase(it); 
            continue;
        }

        // Nếu đạn đã inactive từ trước, xóa luôn khỏi danh sách và bộ nhớ
        if (!proj->isActive()) {
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
                const float healthBefore = target->getHealth();
                target->takeDamage(proj->getDamage());
                const float actualDamage = std::max(
                    0.f, healthBefore - target->getHealth());
                if (context.effects) {
                    ImpactStyle impactStyle = ImpactStyle::Physical;
                    if (proj->getStyle() == ProjectileStyle::Magic) {
                        impactStyle = ImpactStyle::Magic;
                    } else if (proj->getStyle() == ProjectileStyle::Spirit) {
                        impactStyle = ImpactStyle::Spirit;
                    } else if (proj->getStyle() ==
                               ProjectileStyle::HeavySpirit) {
                        impactStyle = ImpactStyle::Strong;
                        context.effects->requestScreenShake(3.5f);
                    } else if (proj->getStyle() ==
                               ProjectileStyle::ExplosiveArrow) {
                        impactStyle = ImpactStyle::Strong;
                    } else if (proj->getShooterTeam() == Team::Enemy) {
                        impactStyle = ImpactStyle::Enemy;
                    }
                    context.effects->spawnImpact(
                        proj->getPosition(), impactStyle);
                    if (actualDamage > 0.f) {
                        context.effects->spawnFloatingNumber(
                            target->getPosition(), actualDamage, false,
                            proj->isStrongAttack());
                    }
                }

                if (proj->hasExplosion()) {
                    const sf::Vector2f impactPosition = target->getPosition();
                    const float radiusSquared =
                        proj->getSplashRadius() * proj->getSplashRadius();
                    for (Entity* nearby : targets) {
                        if (!nearby || nearby == target || nearby->isDead()) {
                            continue;
                        }
                        if (nearby->getTeam() == proj->getShooterTeam()) {
                            continue;
                        }
                        const sf::Vector2f offset =
                            nearby->getPosition() - impactPosition;
                        if (offset.x * offset.x + offset.y * offset.y <=
                            radiusSquared) {
                            const float nearbyHealth = nearby->getHealth();
                            nearby->takeDamage(proj->getSplashDamage());
                            if (context.effects) {
                                context.effects->spawnImpact(
                                    nearby->getPosition(),
                                    ImpactStyle::Strong);
                                const float dealt = std::max(
                                    0.f, nearbyHealth - nearby->getHealth());
                                if (dealt > 0.f) {
                                    context.effects->spawnFloatingNumber(
                                        nearby->getPosition(), dealt,
                                        false, true);
                                }
                            }
                        }
                    }
                }
                
                proj->deactivate(); // Đánh dấu đạn đã trúng mục tiêu
                break; // Trúng 1 mục tiêu là dừng vòng lặp check quái
            }
        }

        // 3. Kiểm tra lại lần nữa sau khi update/va chạm: nếu đạn đã chết (deactivate) thì xóa
        if (!proj->isActive()) {
            it = context.projectiles.erase(it); // Xóa khỏi vector và nhận iterator mới an toàn
        } else {
            ++it; // Chỉ tăng iterator khi không có phần tử nào bị xóa
        }
    }

}
