#include "Projectiles.h" 

#include "Entity.h"
#include "GameContext.h"
#include "Map.h"
#include "MathUtils.h"

#include <cmath>
#include <algorithm>
#include <cstdint>


Projectiles::Projectiles() = default;

Projectiles::Projectiles(
    sf::Vector2f startPosition,
    sf::Vector2f travelDirection,
    float projectileSpeed,
    float projectileDamage,
    Team team,
    ProjectileStyle projectileStyle
)
    : position(startPosition),
      direction(travelDirection),
      speed(projectileSpeed),
      damage(projectileDamage),
      shooterTeam(team),
      style(projectileStyle) {
    // Viết logic khởi tạo các thuộc tính của projectile ở đây (nếu có)
    shape.setSize(sf::Vector2f(24.f, 6.f));
    shape.setOrigin(sf::Vector2f(12.f, 3.f));
    shape.setFillColor(sf::Color::Yellow);
    shape.setPosition(position);
    
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length != 0.f) {
        velocity = (direction / length) * speed;
    } else {
        velocity = sf::Vector2f(0.f, 0.f);
    }

    // Rotate projectile to match velocity angle
    float angleRad = std::atan2(velocity.y, velocity.x);
    float angleDeg = angleRad * 180.f / 3.14159265358979323846f;
    
    shape.setRotation(sf::degrees(angleDeg));
    trailPositions.push_back(position);
}

void Projectiles :: update(const GameContext& context){

    if(!active || context.paused) return ;

    position += velocity * context.deltaTime ; 

    trailTimer -= context.deltaTime;
    if (trailTimer <= 0.f) {
        trailTimer = (style == ProjectileStyle::Magic ||
                      style == ProjectileStyle::Spirit) ? 0.025f : 0.04f;
        trailPositions.push_front(position);
        const std::size_t maxTrail =
            (style == ProjectileStyle::Magic ||
             style == ProjectileStyle::Spirit) ? 9u
            : (style == ProjectileStyle::ExplosiveArrow ? 8u : 6u);
        while (trailPositions.size() > maxTrail) {
            trailPositions.pop_back();
        }
    }

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

void Projectiles::configureVisual(sf::Vector2f size, sf::Color color) {
    shape.setSize(size);
    shape.setOrigin(size / 2.f);
    shape.setFillColor(color);
}

void Projectiles::configureExplosion(float damage, float radius) {
    splashDamage = std::max(0.f, damage);
    splashRadius = std::max(0.f, radius);
    if (hasExplosion()) {
        shape.setFillColor(sf::Color(255, 105, 35));
    }
}

void Projectiles::draw(sf::RenderWindow& window) const {
    const sf::Color trailColor = style == ProjectileStyle::Magic
        ? sf::Color(181, 70, 255)
        : (style == ProjectileStyle::Spirit
            ? sf::Color(72, 235, 205)
            : sf::Color(255, 202, 90));
    const float count = static_cast<float>(
        std::max<std::size_t>(1u, trailPositions.size()));

    if (style == ProjectileStyle::Spirit && trailPositions.size() > 1) {
        for (std::size_t index = 1; index < trailPositions.size(); ++index) {
            const sf::Vector2f start = trailPositions[index];
            const sf::Vector2f delta = trailPositions[index - 1] - start;
            const float length = std::sqrt(delta.x * delta.x + delta.y * delta.y);
            if (length <= 0.01f) continue;
            const float strength = 1.f - static_cast<float>(index) / count;
            sf::RectangleShape ribbon({length, 3.f + strength * 2.f});
            ribbon.setOrigin({0.f, ribbon.getSize().y / 2.f});
            ribbon.setPosition(start);
            ribbon.setRotation(sf::radians(std::atan2(delta.y, delta.x)));
            ribbon.setFillColor(index % 2 == 0
                ? sf::Color(72, 235, 205,
                    static_cast<std::uint8_t>(35.f + strength * 100.f))
                : sf::Color(255, 222, 125,
                    static_cast<std::uint8_t>(30.f + strength * 80.f)));
            window.draw(ribbon);
        }
    }

    for (std::size_t index = trailPositions.size(); index-- > 0;) {
        const float strength = 1.f - static_cast<float>(index) / count;
        if (style == ProjectileStyle::Magic ||
            style == ProjectileStyle::Spirit) {
            const float radius = 2.f + strength * 4.5f;
            sf::CircleShape trail(radius);
            trail.setOrigin({radius, radius});
            sf::Vector2f trailPosition = trailPositions[index];
            sf::Color pointColor = trailColor;
            if (style == ProjectileStyle::Spirit) {
                const sf::Vector2f normal{-direction.y, direction.x};
                trailPosition += normal * std::sin(
                    distanceTraveled * 0.07f +
                    static_cast<float>(index) * 1.25f) * 3.5f;
                if (index % 2 == 1) pointColor = sf::Color(255, 220, 115);
            }
            trail.setPosition(trailPosition);
            trail.setFillColor(sf::Color(
                pointColor.r, pointColor.g, pointColor.b,
                static_cast<std::uint8_t>(30.f + strength * 105.f)));
            window.draw(trail);
        } else {
            const float width = style == ProjectileStyle::ExplosiveArrow
                ? 3.5f : 2.f;
            sf::RectangleShape trail({7.f + strength * 8.f, width});
            trail.setOrigin({7.f, 1.f});
            trail.setPosition(trailPositions[index]);
            trail.setRotation(shape.getRotation());
            trail.setFillColor(sf::Color(
                trailColor.r, trailColor.g, trailColor.b,
                static_cast<std::uint8_t>(25.f + strength * 95.f)));
            window.draw(trail);
        }
    }

    if (style == ProjectileStyle::Magic ||
        style == ProjectileStyle::Spirit) {
        const float glowRadius =
            (style == ProjectileStyle::Spirit ? 12.f : 10.f) +
            std::sin(distanceTraveled * 0.08f) * 2.f;
        sf::CircleShape glow(glowRadius);
        glow.setOrigin({glowRadius, glowRadius});
        glow.setPosition(position);
        glow.setFillColor(style == ProjectileStyle::Spirit
            ? sf::Color(255, 224, 125, 78)
            : sf::Color(166, 64, 255, 72));
        window.draw(glow);
    } else {
        sf::RectangleShape glow({28.f, 10.f});
        glow.setOrigin({14.f, 5.f});
        glow.setPosition(position);
        glow.setRotation(shape.getRotation());
        glow.setFillColor(sf::Color(255, 205, 90, 45));
        window.draw(glow);
    }
    if (style == ProjectileStyle::Spirit) {
        sf::CircleShape rim(9.f);
        rim.setOrigin({9.f, 9.f});
        rim.setPosition(position);
        rim.setFillColor(sf::Color(255, 229, 145, 225));
        window.draw(rim);
        sf::CircleShape core(6.5f);
        core.setOrigin({6.5f, 6.5f});
        core.setPosition(position);
        core.setFillColor(sf::Color(87, 255, 220));
        window.draw(core);
    } else {
        window.draw(shape);
    }
}
