#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_set>

class Entity;
class CombatManager;
class GameContext;

class Weapon {
private:
    std::unordered_set<Entity*> hitList;

protected:
    bool hasAttacked = false;

public:
    virtual ~Weapon() = default;

    // === PURE VIRTUAL – BẮT BUỘC LỚP CON IMPLEMENT ===
    virtual sf::FloatRect getHitbox(sf::Vector2f entityCenter, sf::Vector2f attackDir) = 0;
    virtual bool isHitting(sf::Vector2f attackerPos, sf::Vector2f attackDir, sf::Vector2f targetPos) = 0;
    virtual void setDamage(int damage) = 0;
    virtual int getDamage() const = 0;
    virtual void triggerAction(Entity* attacker, GameContext& context, CombatManager& combatManager) = 0;

    // === QUẢN LÝ HITLIST ===
    std::unordered_set<Entity*> getHitList() const { return hitList; }
    bool isHit(Entity* target) const { return hitList.find(target) != hitList.end(); }
    void addHit(Entity* target) { hitList.insert(target); }
    void forgetEntity(const Entity* target) {
        hitList.erase(const_cast<Entity*>(target));
    }
    void clearHitList() { hitList.clear(); }
    std::size_t getHitCount() const { return hitList.size(); }

    // === TRẠNG THÁI TẤN CÔNG ===
    bool getHasAttacked() const { return hasAttacked; }
    void setHasAttacked(bool val) { hasAttacked = val; }

    // === DEBUG ===
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
