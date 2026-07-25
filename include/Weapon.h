#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_set>

class Entity;
class CombatManager ; 
class GameContext ; 
class Weapon {
private:
    std::unordered_set<Entity*> hitList;
    
protected : 
    
    bool hasAttacked = false ; //bool nay de dung cho class bow va sau nay co the dung de toi uu sword

public:
    virtual ~Weapon() = default;

    virtual sf::FloatRect getHitbox(sf::Vector2f entityCenter, sf::Vector2f attackDir) ;
    virtual bool isHitting(sf::Vector2f attackerPos, sf::Vector2f attackDir, sf::Vector2f targetPos) ;

    virtual void setDamage(int damage) = 0;
    virtual int getDamage() const = 0;

    // Quản lý hitList
    std::unordered_set<Entity*> getHitList() const { return hitList; }
    bool isHit(Entity* target) const { return hitList.find(target) != hitList.end(); }
    void addHit(Entity* target) { hitList.insert(target); }
    void clearHitList() { hitList.clear(); }

    virtual void drawDebug(sf::RenderTarget& target, sf::Vector2f entityCenter, sf::Vector2f attackDir) {
        sf::FloatRect hitbox = getHitbox(entityCenter, attackDir);
        sf::RectangleShape debugRect;
        debugRect.setPosition(hitbox.position);
        debugRect.setSize(hitbox.size);
        debugRect.setFillColor(sf::Color(255, 0, 0, 50));
        debugRect.setOutlineColor(sf::Color::Red);
        debugRect.setOutlineThickness(1.5f);
        target.draw(debugRect);
    }

    virtual void triggerAction(Entity* attacker,GameContext& context, CombatManager& combatManager) = 0 ; 

    bool getHasAttacked()const {return this->hasAttacked ;}
    void setHasAttacked(bool val){this->hasAttacked = val ;}
};