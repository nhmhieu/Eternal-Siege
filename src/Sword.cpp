#include "Sword.h"
#include "GameContext.h" 
#include "CombatManager.h"
#include "Effects.h"
#include "Ally.h"
#include "Monster.h"
#include "AudioManager.h"
// #include "GameTypes.h"


sf::FloatRect Sword :: getHitbox(sf::Vector2f entityCenter, sf::Vector2f attackDir){
    // 1. Độ dày của nhát chém (kiếm chém thường quét 1 vùng rộng)
    constexpr float thickness = 40.f; 

    // 2. Kích thước Hitbox tự động co giãn theo hướng đánh
    // Nếu đánh ngang (x=1, y=0) -> width dài, height ngắn
    // Nếu đánh chéo (x=0.7, y=0.7) -> tạo ra 1 ô vuông bao bọc đường chéo
    float width = std::abs(attackDir.x * range) + thickness;
    float height = std::abs(attackDir.y * range) + thickness;

    // 3. Tâm của nhát chém: Nằm ở CHÍNH GIỮA khoảng cách từ nhân vật đến mũi kiếm
    sf::Vector2f hitboxCenter = entityCenter + (attackDir * (range / 2.f));

    // 4. Tìm góc trên-trái (Top-Left) để nạp vào sf::FloatRect
    sf::Vector2f pos = hitboxCenter - sf::Vector2f(width / 2.f, height / 2.f);

    return sf::FloatRect(pos, sf::Vector2f(width, height));
}

bool Sword :: isHitting(sf::Vector2f attackerPos, sf::Vector2f attackDir, sf::Vector2f targetPos){
    sf::Vector2f dirToTarget = targetPos - attackerPos;
    float distance = std::sqrt(dirToTarget.x * dirToTarget.x + dirToTarget.y * dirToTarget.y);
    if (distance > range) return false;
    if (distance < 0.0001f) return true;

    sf::Vector2f normalizedDir = dirToTarget / distance;
    float dot = (attackDir.x * normalizedDir.x) + (attackDir.y * normalizedDir.y);

    constexpr float cosHalfAngle = 0.70710678f; // cos(45°) cho góc quạt 90°
    return dot >= cosHalfAngle;
}

void Sword :: triggerAction(Entity* attacker, GameContext& context, CombatManager& combatManager){
    if (!attacker || attacker->isDead() ||
        !attacker->getIsAttacking() || hasAttacked) {
        return;
    }
    if (context.effects) {
        if (const auto* ally = dynamic_cast<const Ally*>(attacker)) {
            if (ally->getType() == AllyType::Lucas) {
                context.effects->spawnSlash(
                    attacker->getPosition(), attacker->getAttackDirection(),
                    sf::Color(245, 82, 35), 1.25f);
            } else {
                context.effects->spawnSlash(
                    attacker->getPosition(), attacker->getAttackDirection(),
                    sf::Color(110, 225, 255), 0.92f);
            }
        } else if (const auto* monster = dynamic_cast<const Monster*>(attacker)) {
            const float scale = monster->isBoss() ? 1.45f
                : (monster->isElite() ? 1.2f : 0.9f);
            context.effects->spawnSlash(
                attacker->getPosition(), attacker->getAttackDirection(),
                monster->isBoss() ? sf::Color(225, 70, 170)
                                  : sf::Color(255, 100, 92), scale);
        } else {
            context.effects->spawnSlash(
                attacker->getPosition(), attacker->getAttackDirection(),
                attacker->getTeam() == Team::Enemy);
        }
    }
    if(attacker->getTeam() == Team :: Player){
        combatManager.processAttack(
            attacker, this, context.enemies, context.effects) ;

    }
    else if(attacker->getTeam() == Team :: Enemy){
        combatManager.processAttack(
            attacker, this, context.players, context.effects) ;
    }
    if (context.audioManager && dynamic_cast<const Ally*>(attacker)) {
        context.audioManager->playSound("melee_hit");
    }
    hasAttacked = true;
}
