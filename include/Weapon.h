#pragma once 

#include <SFML/Graphics.hpp> 

class Weapon{

    public : 

    //destructor 
    virtual ~Weapon() = default ; 


    //Moi vu khi co cach ve hitbox cua rieng minh
    virtual sf :: FloatRect getHitbox(sf :: Vector2f entityCenter, sf :: Vector2f attackDir) = 0 ; 
    virtual bool isHitting(sf::Vector2f attackerPos, sf::Vector2f attackDir, sf::Vector2f targetPos) = 0;

    // getter/setter 
    virtual void setDamage(int damage) = 0 ; 
    virtual int getDamage() const = 0 ; 


} ; 