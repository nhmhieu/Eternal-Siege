#pragma once 
#include <SFML/Graphics.hpp>
#include <deque>
#include "Entity.h"
#include "GameTypes.h"

class GameContext;

enum class ProjectileStyle {
    Arrow,
    Magic,
    Spirit,
    ExplosiveArrow
};

class Projectiles{ 

private : 

    sf::Vector2f position{};
    sf::Vector2f direction{};
    sf::Vector2f velocity{};
    float speed = 0.f;
    float damage = 0.f;
    Team shooterTeam = Team::Neutral;
    bool active = true ; 
    float maxRange = 800.f ; 
    float distanceTraveled = 0.f ; 
    float splashDamage = 0.f;
    float splashRadius = 0.f;
    ProjectileStyle style = ProjectileStyle::Arrow;
    std::deque<sf::Vector2f> trailPositions;
    float trailTimer = 0.f;

    sf :: RectangleShape shape ; 

protected:
    void configureVisual(sf::Vector2f size, sf::Color color);

public : 

    Projectiles() ; 
    virtual ~Projectiles() = default ;
    Projectiles(sf::Vector2f startPosition,
         sf::Vector2f direction, float speed,
         float damage, Team shooterTeam,
         ProjectileStyle style = ProjectileStyle::Arrow);
    void update(const GameContext& context) ; 
    void draw(sf::RenderWindow& window) const;
    sf :: FloatRect getBounds() const {return shape.getGlobalBounds() ;}

    bool isActive() const { return active; }
    void deactivate() { active = false; } //ham tat projectile
    float getDamage() const { return damage; }
    Team getShooterTeam() const { return shooterTeam; }
    sf::Vector2f getPosition() const { return position; }
    ProjectileStyle getStyle() const { return style; }
    bool isStrongAttack() const {
        return style == ProjectileStyle::ExplosiveArrow;
    }
    std::size_t getTrailPointCount() const { return trailPositions.size(); }
    bool hasExplosion() const {
        return splashDamage > 0.f && splashRadius > 0.f;
    }
    float getSplashDamage() const { return splashDamage; }
    float getSplashRadius() const { return splashRadius; }
    void configureExplosion(float damage, float radius);
    bool isHitting(Entity* target) ;

    const sf::RectangleShape& getShape() const { return shape; }



} ;
