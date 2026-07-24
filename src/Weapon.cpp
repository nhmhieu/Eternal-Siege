#include "Weapon.h"
#include "SFML/Graphics.hpp"

sf::FloatRect Weapon :: getHitbox(sf::Vector2f entityCenter, sf::Vector2f attackDir){
    return sf::FloatRect({0.f, 0.f}, {0.f, 0.f});
}
bool Weapon :: isHitting(sf::Vector2f attackerPos, sf::Vector2f attackDir, sf::Vector2f targetPos){
    return false ; 
}
