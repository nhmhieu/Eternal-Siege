#include "Sword.h"
#include "GameContext.h" 
#include "CombatManager.h"
#include <iostream>
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
    if (distance <= 0.0001f) return true;

    sf::Vector2f normalizedDir = dirToTarget / distance;
    float dot = (attackDir.x * normalizedDir.x) + (attackDir.y * normalizedDir.y);

    constexpr float cosHalfAngle = 0.70710678f; // cos(45°) cho góc quạt 90°
    return dot >= cosHalfAngle;
}

void Sword :: triggerAction(Entity* attacker, GameContext& context, CombatManager& combatManager){
    if(attacker->getTeam() == Team :: Player){
        combatManager.processAttack(attacker, this, context.enemies) ; 

    }
    else if(attacker->getTeam() == Team :: Enemy){
        combatManager.processAttack(attacker, this, context.players) ; 
    }
    else{
        std :: cout << "Truong hop ngoai le (Neutral) !!" << std :: endl ; 
    }
}
