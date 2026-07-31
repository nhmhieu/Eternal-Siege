#include "Projectiles.h" 

#include "Entity.h"
#include "GameContext.h"
#include "Map.h"
#include "MathUtils.h"

#include <cmath>


Projectiles::Projectiles() = default;

Projectiles::Projectiles(
    sf::Vector2f startPosition,
    sf::Vector2f travelDirection,
    float projectileSpeed,
    float projectileDamage,
    Team team
)
    : position(startPosition),
      direction(travelDirection),
      speed(projectileSpeed),
      damage(projectileDamage),
      shooterTeam(team) {
    // Viết logic khởi tạo các thuộc tính của projectile ở đây (nếu có)
    // Ví dụ:
    // 1. Khởi tạo kích thước hình chữ nhật (Dài x Rộng)
    shape.setSize(sf::Vector2f(24.f, 6.f));
    
    // 2. Đặt tâm xoay về giữa mũi tên
    shape.setOrigin(sf::Vector2f(12.f, 3.f));
    
    // 3. Màu sắc hiển thị thử
    shape.setFillColor(sf::Color::Yellow);
    
    // 4. Đặt vị trí xuất phát
    shape.setPosition(position);
    
    // 5. Chuẩn hóa vector hướng đi
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length != 0.f) {
        velocity = (direction / length) * speed;
    } else {
        velocity = sf::Vector2f(0.f, 0.f);
    }

    // 6. Xoay mũi tên theo hướng bay (ĐIỂM KHÁC BIỆT QUAN TRỌNG TRONG SFML 3)
    float angleRad = std::atan2(velocity.y, velocity.x);
    float angleDeg = angleRad * 180.f / 3.14159265358979323846f;
    
    // Trong SFML 3, bắt buộc phải dùng sf::degrees() thay vì truyền số float đơn thuần
    shape.setRotation(sf::degrees(angleDeg));
}

void Projectiles :: update(const GameContext& context){

    if(!active) return ; 

    position += velocity * context.deltaTime ; 

    // Dan khong duoc bay xuyen tuong.
    if (context.map && !context.map->isWalkableWorld(position, 3.f)) {
        active = false;
        return;
    }

    //bay ra khoi tam toi da roi thi tat active
    distanceTraveled += speed * context.deltaTime ; 
    if(distanceTraveled >= maxRange){
        active = false ; 
        return  ; 
    }

    //dong bo vi tri voi sprite 
    shape.setPosition(position) ; 

    //tinh toan goc cua sprite se huong toi (bay huong ve dau thi quay mat ve do)
    shape.setRotation(sf::radians(std::atan2(direction.y, direction.x)));


}

bool Projectiles :: isHitting(Entity* target){

    if(!isActive()) return false ; 
    if(!target || target->isDead()) return false ; 

    return getBounds().findIntersection(target->getHurtBox()).has_value() ; 

}
