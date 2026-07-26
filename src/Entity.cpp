#include "Entity.h"
#include "GameContext.h"
#include <iostream>

Entity::Entity() : position(0, 0), health(0), maxHealth(0) {}

Entity::Entity(float x, float y, float health, float maxHealth)
    : position(x, y), health(health), maxHealth(maxHealth) {
}

Entity::Entity(float x, float y, float health, float maxHealth, Team team, Weapon* weapon)
    : position(x, y), health(health), maxHealth(maxHealth), team(team), currentWeapon(weapon) {
}

Entity :: ~Entity(){
    delete this->currentWeapon ; 
}

void Entity::takeDamage(float damage) {
    health -= damage;
    if (health <= 0) {
        health = 0;
        isAlive = false;
        std::cout << "Entity hp is 0 !" << std::endl;
    }
}

sf::FloatRect Entity::getCollisionBox() const {
    return sprite.getGlobalBounds();
}

sf::FloatRect Entity::getHurtBox() const {
    return sprite.getGlobalBounds();
}

sf::FloatRect Entity::getAttackHitbox() const {
    sf::FloatRect bounds = sprite.getGlobalBounds();
    sf::Vector2f center = bounds.position + (bounds.size / 2.f);
    float range = 40.f;
    sf::Vector2f hitboxSize(40.f, 40.f);
    sf::Vector2f attackCenter = center + (attackDirection * range);
    sf::Vector2f attackPos = attackCenter - (hitboxSize / 2.f);
    return sf::FloatRect(attackPos, hitboxSize);
}

void Entity::updateStatus() {
    if (isAttacking) {
        if (attackTimer >= attackDuration) {
            isAttacking = false;
            if (currentWeapon){
                currentWeapon->clearHitList();
                currentWeapon->setHasAttacked(false) ;

            }
            coolDownTimer = attackCoolDown ; 
            attackTimer = 0.f ; 


        }
    }
}

void Entity :: updateDeadTimer(const GameContext& context){

        if(isDying){
        this->deadTimer += context.deltaTime ;
        //logic animation quai khi chet -----
        return ;
    }
}

void Entity :: updateAttackTimer(const GameContext& context){
    if(isAttacking){
        attackTimer += context.deltaTime ; 

    }
}


void Entity :: update(GameContext& context){

    

}