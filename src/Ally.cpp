#include "Ally.h"
#include "Constants.h"
#include <iostream>

using namespace GameConfig;

Ally::Ally(float startX, float startY)
{
    // 1. Cài đặt hình ảnh/màu sắc cho Tướng
    // Thêm cặp ngoặc nhọn { } vào setSize
    shape.setSize({TILE_SIZE - 12.f, TILE_SIZE - 12.f});
    shape.setFillColor(sf::Color::Cyan); 
    shape.setOutlineColor(sf::Color::White);
    shape.setOutlineThickness(1.f);
    
    // Thêm cặp ngoặc nhọn { } vào setOrigin và setPosition
    shape.setOrigin({shape.getSize().x / 2.f, shape.getSize().y / 2.f});
    shape.setPosition({startX, startY});

    // 2. Cài đặt vòng tròn hiển thị tầm đánh
    rangeCircle.setRadius(attackRange);
    rangeCircle.setFillColor(sf::Color(0, 255, 255, 25)); 
    rangeCircle.setOutlineColor(sf::Color(0, 255, 255, 100));
    rangeCircle.setOutlineThickness(1.f);
    
    // Thêm cặp ngoặc nhọn { } vào setOrigin và setPosition
    rangeCircle.setOrigin({attackRange, attackRange});
    rangeCircle.setPosition({startX, startY});
}

void Ally::update(float dt)
{
    if (attackCooldown > 0.f)
    {
        attackCooldown -= dt;
    }
}

void Ally::render(sf::RenderWindow& window)
{
    window.draw(rangeCircle); 
    window.draw(shape);       
}

sf::Vector2f Ally::getPosition() const
{
    return shape.getPosition();
}

float Ally::getAttackRange() const
{
    return attackRange;
}