#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_set>

class Entity;

class Weapon {
private:
    std::unordered_set<Entity*> hitList;

public:
    virtual ~Weapon() = default;

    virtual sf::FloatRect getHitbox(sf::Vector2f entityCenter, sf::Vector2f attackDir) = 0;
    virtual bool isHitting(sf::Vector2f attackerPos, sf::Vector2f attackDir, sf::Vector2f targetPos) = 0;

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
};