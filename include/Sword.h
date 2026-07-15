#include "Weapon.h"

#include <cmath> 

class Sword : public Weapon {
private:
    int damage;
    float range;

public:
    //constructor
    Sword(int dmg, float r) : damage(dmg), range(r) {}

    // Override lại hàm từ lớp cha
    sf::FloatRect getHitbox(sf::Vector2f entityCenter, sf::Vector2f attackDir) override {
    // 1. Độ rộng/cao của hộp quét (ví dụ: ngang 100px để bao phủ cả chiều dọc)
    // Chiều dài của hộp (theo hướng chém) lúc này chính là tầm đánh (range = 150)
    sf::Vector2f size(range, 100.f); 
    
    // 2. Chỉ dịch chuyển tâm của hộp đi một khoảng bằng NỬA tầm đánh (range / 2)
    // Việc này giúp rìa sau của hộp luôn dính sát vào cơ thể Player, 
    // còn rìa trước của hộp sẽ vươn xa tới đúng mốc 150px.
    float pushDistance = range / 2.0f; 
    
    sf::Vector2f pos = entityCenter + (attackDir * pushDistance) - (size / 2.f);
    
    return sf::FloatRect(pos, size);
}

    bool isHitting(sf::Vector2f attackerPos, sf::Vector2f attackDir, sf::Vector2f targetPos) {
        // 1. Vector từ nhân vật đến quái
        sf::Vector2f V = targetPos - attackerPos;
        
        // 2. Tính khoảng cách (độ dài vector V)
        float distance = std::sqrt(V.x * V.x + V.y * V.y);
        
        // Kiểm tra phạm vi (range)
        if (distance > this->range) return false;
        
        // 3. Chuẩn hóa vector V để tính góc (chia cho độ dài)
        sf::Vector2f V_norm = V / distance;
        
        // 4. Tính Cos góc lệch (Dot Product)
        float dotProduct = (attackDir.x * V_norm.x) + (attackDir.y * V_norm.y);
        
        // 5. Tính góc theta (theta = arccos(dotProduct))
        // Để hiệu năng cao, ta không cần gọi acos, ta so sánh trực tiếp với cos(góc_giới_hạn)
        // Ví dụ: góc quét là 90 độ, thì nửa góc là 45 độ. cos(45 độ) ~ 0.707
        float cosTheta = std::cos(45.0f * 3.14159f / 180.0f); // ~ 0.707
        
        return dotProduct >= cosTheta;
    }

    void setDamage(int dmg) override { damage = dmg; }
    int getDamage() const override { return damage; }
};