#include "Bow.h"
#include "Arrow.h" 
#include "CombatManager.h"

Bow :: Bow(){

}

Bow :: Bow(float dmg, float cooldown){
    this->damage = dmg ; 
    this->cooldown = cooldown ; 

}

Bow :: ~Bow(){

}

void Bow :: fire(sf :: Vector2f position, sf :: Vector2f direction, Team shooterTeam, GameContext& context){

    Arrow* newArrow = new Arrow(position, direction, projectTileSpeed,
    damage, shooterTeam) ; 

    context.projectiles.push_back(newArrow) ; 

}

void Bow::setDamage(int dmg) {
    damage = dmg; // Gán giá trị sát thương
}

int Bow::getDamage() const {
    return damage; // Trả về giá trị sát thương
}

void Bow :: triggerAction(Entity* attacker, GameContext& context, CombatManager& combatManager){
    this->fire(attacker->getPosition(), attacker->getDirection(), attacker->getTeam(),
context) ;
}
