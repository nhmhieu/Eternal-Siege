#pragma once

#include "Weapon.h"
#include "GameTypes.h"

class GameContext ;
class CombatManager ; 
class Bow : public Weapon{

    private : 
        float projectTileSpeed = 500.f ; 
        static constexpr float attackRange = 250.f;

    public : 
        Bow() = default;
        ~Bow() override = default;

        void fire(sf :: Vector2f position, sf :: Vector2f direction, 
        Team shooterTeam, float damage, GameContext& context) ;

        sf::FloatRect getHitbox(sf::Vector2f entityCenter, sf::Vector2f attackDir) override;
        bool isHitting(sf::Vector2f attackerPos, sf::Vector2f attackDir, sf::Vector2f targetPos) override;
        float getAttackRange() const override { return attackRange; }
        void triggerAction(Entity* attacker, GameContext& context, CombatManager& combatManager) override;

} ;
