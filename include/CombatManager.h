#pragma once
#include <vector>
#include <set>
#include "Entity.h"
#include "Weapon.h"

class CombatManager {
private:
    //Cac Entities o trong day da nhan sat thuong va khong can tru mau nua
    std::set<Entity*> hitEntities;

public:
    // Xử lý logic tấn công
    void processAttack(Entity* attacker, Weapon* weapon, std::vector<Entity*>& targets);

    // Gọi hàm này khi kết thúc animation tấn công để reset lại danh sách hitEntities
    void resetAttack();
};