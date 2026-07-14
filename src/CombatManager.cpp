#include "CombatManager.h"

void CombatManager::processAttack(Entity* attacker, Weapon* weapon, std::vector<Entity*>& targets) {

    //edge case 
    if(!attacker || !weapon ) return ; 
    if (!attacker->getIsAttacking()) return;

    // Thêm dòng này để kiểm tra vị trí Player
// std::cout << "Vi tri Player LOGIC: (" << attacker->getX() << ", " << attacker->getY() << ")" << std::endl;

    //Lay hitbox cua vu khi dang duoc attacker su dung hien tai
    sf::FloatRect attackArea = weapon->getHitbox(
        {attacker->getX(), attacker->getY()}, 
        attacker->getAttackDirection()
    );

// CHÈN VÀO NGAY DƯỚI DÒNG LẤY attackArea
std::cout << "--- HITBOX ATTACK AREA (SFML 3) ---" << std::endl;
std::cout << "Vi tri (X, Y): (" << attackArea.position.x << ", " << attackArea.position.y << ")" << std::endl;
std::cout << "Kich thuoc (W, H): (" << attackArea.size.x << ", " << attackArea.size.y << ")" << std::endl;
std::cout << "-----------------------------------" << std::endl;

    std :: cout << "kiem tra entity voi hitbox" << endl ; 
    if(targets.empty()) cout << "Targets dang trong !!!" << endl ; 
    for (Entity* target : targets) {


        // IN THỬ HURTBOX CỦA QUÁI VẬT
    sf::FloatRect hurtBox = target->getHurtBox();
    std::cout << "--- QUAI VAT HURTBOX ---" << std::endl;
    std::cout << "Vi tri quai (X, Y): (" << hurtBox.position.x << ", " << hurtBox.position.y << ")" << std::endl;
    std::cout << "Kich thuoc quai (W, H): (" << hurtBox.size.x << ", " << hurtBox.size.y << ")" << std::endl;


        if(target->getTeam() == attacker->getTeam()){
            cout << "Target va attacker cung phe" << endl ;
            continue; //bo qua neu cung phe 
        }
 
        // 3. Kiểm tra va chạm (Broad-phase)
        if (attackArea.findIntersection(target->getHurtBox()).has_value()){
            
            // 4. Kiểm tra va chạm chính xác (Narrow-phase - Hình quạt)
            if (weapon->isHitting({attacker->getX(), attacker->getY()}, 
                                  attacker->getAttackDirection(), 
                                  {target->getX(), target->getY()})) {
                
                // 5. Nếu chưa trúng đòn này thì trừ máu
                if (hitEntities.find(target) == hitEntities.end()) {
                    //tru mau o day
                    target->takeDamage(attacker->getAttackPower());
                    hitEntities.insert(target); // Đánh dấu đã trúng
                }
            }
        }
    }
}

void CombatManager::resetAttack() {
    hitEntities.clear();
}