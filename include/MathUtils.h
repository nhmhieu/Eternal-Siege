#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <SFML/System/Vector2.hpp>
#include <cmath>

// Dùng inline để tránh lỗi "multiple definition" khi include ở nhiều file
inline float getDistanceSquared(sf::Vector2f a, sf::Vector2f b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return (dx * dx + dy * dy);
}

// Nếu cần dùng khoảng cách thường (có căn bậc hai)
inline float getDistance(sf::Vector2f a, sf::Vector2f b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

#endif