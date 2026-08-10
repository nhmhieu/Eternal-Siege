#pragma once
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
struct KingdomFootModel{
 static sf::FloatRect playerCollider(sf::Vector2f p){return{{p.x-14,p.y-8},{28,16}};}
 static sf::FloatRect npcCollider(sf::Vector2f p){return{{p.x-10,p.y-6},{20,12}};}
};
