#include "Projectiles.h" 
#include "MathUtils.h"
#include "GameContext.h"


Projectiles :: Projectiles(){

    position.x = 0 ; 
    position.y = 0 ; 
    speed = 0 ; 
    direction = {0, 0} ; 
    damage = 0 ; 
    // shooterTeam = Neutral ; 
}

Projectiles::Projectiles(sf::Vector2f position, sf::Vector2f direction, float speed, float damage, Team team) {
    // Viết logic khởi tạo các thuộc tính của projectile ở đây (nếu có)
    // Ví dụ:
    this->position = position;
    this->direction = direction;
    this->speed = speed;
    this->damage = damage;
    this->shooterTeam = team;
}

void Projectiles :: update(const GameContext& context){

    if(!active) return ; 

    position += position * speed * context.deltaTime ; 

    //bay ra khoi tam toi da roi thi tat active
    distanceTraveled += speed * context.deltaTime ; 
    if(distanceTraveled >= maxRange){
        active = false ; 
        return  ; 
    }

    //dong bo vi tri voi sprite 
    sprite.setPosition(position) ; 

    //tinh toan goc cua sprite se huong toi (bay huong ve dau thi quay mat ve do)
    sprite.setRotation(sf::radians(std::atan2(direction.y, direction.x)));


}

bool Projectiles :: isHitting(Entity* target){

    if(!isActive()) return false ; 
    if(!target || target->isDead()) return false ; 

    return getBounds().findIntersection(target->getHurtBox()).has_value() ; 

}