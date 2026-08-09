#pragma once
#include "AnimationController.h"
#include "KingdomMap.h"
#include <SFML/Graphics.hpp>
#include <vector>

enum class KingdomNpcRole { Guard, Citizen };

class KingdomNpc {
public:
    KingdomNpc(KingdomNpcRole role,const sf::Texture& texture,
               std::vector<sf::Vector2f> route,float speed=72.f);
    void update(float dt,const KingdomMap& map);
    void drawShadow(sf::RenderWindow&)const;
    void draw(sf::RenderWindow&)const;
    float footY()const{return position.y;}
    sf::Vector2f getPosition()const{return position;}
    sf::Vector2f getFootPosition()const{return position;}
    sf::FloatRect getFootCollider()const{return {{position.x-10.f,position.y-6.f},{20.f,12.f}};}
    void setTint(sf::Color color){sprite.setColor(color);}
    KingdomNpcRole getRole()const{return role;}
    const std::vector<sf::Vector2f>& getRoute()const{return route;}
private:
    KingdomNpcRole role;const sf::Texture* texture;sf::Sprite sprite;
    AnimationController animation;std::vector<sf::Vector2f> route;
    sf::Vector2f position;std::size_t target=0;float speed;float wait=0.f;
    FacingDirection facing=FacingDirection::Down;
    void setFacing(FacingDirection direction);
};
