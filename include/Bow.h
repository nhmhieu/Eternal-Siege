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

        int getDamage() const override ;
        void setDamage(int damage) override ;
        void triggerAction(Entity* attacker,GameContext& context, CombatManager& combatManager) override ; 

} ;