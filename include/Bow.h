#pragma once

#include "Weapon.h"
#include "GameTypes.h"

class GameContext ;
class CombatManager ; 
class Bow : public Weapon{

    private : 
        int damage = 5 ; 
        float projectTileSpeed = 500.f ; 
        float cooldown = 0 ; 

    public : 
        Bow(float dmg, float cooldown) ; 
        Bow() ; 
        ~Bow() ; 

        void fire(sf :: Vector2f position, sf :: Vector2f direction, 
        Team shooterTeam, GameContext& context) ; 

        sf::FloatRect getHitbox(sf::Vector2f entityCenter, sf::Vector2f attackDir) override;
        bool isHitting(sf::Vector2f attackerPos, sf::Vector2f attackDir, sf::Vector2f targetPos) override;
        int getDamage() const override;
        void setDamage(int damage) override;
        void triggerAction(Entity* attacker, GameContext& context, CombatManager& combatManager) override;

} ;
