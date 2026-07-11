#include "Entity.h" 

Entity :: Entity(){
    x = 0 ;
    y = 0 ;
    health = 0 ;
    maxHealth = 0 ; 
}

Entity :: ~Entity(){

}

Entity :: Entity(float x, float y, float health, float maxHealth){
    this->x = x ;
    this->y = y ;
    this->health = health ;
    this->maxHealth = maxHealth ; 
}

void Entity :: takeDamage(float damage){
    health -= damage ;
    if(health <= 0) health = 0 ; 
}

bool Entity :: isDead() const{
    return true ; 
}
