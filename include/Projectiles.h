#pragma once 
#include <SFML/Graphics.hpp>
#include "Entity.h"
#include "GameTypes.h"

class GameContext;

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

    sf :: RectangleShape shape ; 

public : 

    Projectiles() ; 
    ~Projectiles() = default ;
    Projectiles(sf::Vector2f startPosition,
         sf::Vector2f direction, float speed,
         float damage, Team shooterTeam);
    void update(const GameContext& context) ; 
    sf :: FloatRect getBounds() const {return shape.getGlobalBounds() ;}

    bool isActive() const { return active; }
    void deactivate() { active = false; } //ham tat projectile
    float getDamage() const { return damage; }
    Team getShooterTeam() const { return shooterTeam; }
    bool isHitting(Entity* target) ;

    const sf::RectangleShape& getShape() const { return shape; }



} ;
