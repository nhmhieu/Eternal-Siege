#ifndef ALLY_H
#define ALLY_H

#include <SFML/Graphics.hpp>
// #include "Entity.h" // <-- Mở comment dòng này nếu project của bạn đã tạo class Entity

// Nếu đã có class Entity, bạn đổi thành: class Ally : public Entity
class Ally 
{
public:
    Ally(float startX, float startY);

    void update(float dt);
    void render(sf::RenderWindow& window);

    sf::Vector2f getPosition() const;
    float getAttackRange() const;

private:
    sf::RectangleShape shape;       // Hình dáng của Tướng (sau này thay bằng sf::Sprite)
    sf::CircleShape rangeCircle;    // Vòng tròn tầm đánh (dùng để debug)

    float attackRange = 150.f;      // Tầm đánh (pixel)
    float attackCooldown = 0.f;     // Thời gian chờ giữa 2 đòn đánh
    const float ATTACK_RATE = 1.0f; // 1 giây tấn công 1 lần
};

#endif // ALLY_H