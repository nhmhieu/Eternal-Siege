#include "Ally.h"
#include "GameContext.h"
#include "MathUtils.h"

#include <cmath>
#include <iostream>

Ally::Ally(float x, float y)
    : Entity(x, y, 50.f, 50.f), range(200.f), attackCoolDown(2.f)
{
    sprite.setRadius(20.f);
    sprite.setFillColor(sf::Color::Blue);
    sprite.setOrigin(sf::Vector2f(20.f, 20.f));
    sprite.setPosition(sf::Vector2f(x, y));
    team = Team::Player;
    
    this->target = nullptr ; //Muc tieu ban dau la null
    isAttacking = false ;
    range = 200.f ; 
    attackDuration = 2.f ;
    coolDownTimer = 0.f ; 
}

void Ally::update(GameContext& context) {
    if(coolDownTimer > 0.f) 
        coolDownTimer -= context.deltaTime ; 

        //cap nhat target gan nhat hien co trong context
        updateTarget(context); 


    if(target && !target->isDead()){
        //nap huong tan cong moi nhat va truyen vao attribute 

        sf::Vector2f dir = target->getPosition() - this->getPosition(); 
        
        float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        
        if (length != 0.f) {
            dir /= length;
        } else {
            dir = { 1.f, 0.f }; 
        }
        
        this->setAttackDirection(dir);

    }

    if(isDying){
        updateDeadTimer(context) ; 
    }

    if(
        (target) 
        && (!target->isDead()) 
        // && (canAttack()) //dang bi loi o day
    )
        {
        startAttacking() ; 
    }
    else{
    }

    if(isAttacking){
        updateAttackTimer(context) ; 
        getCurrentWeapon()->triggerAction(this, context, *context.combatManager) ; 
        // std :: cout << "Allies attacked, enemy current health : " << target->getHealth() << std :: endl ;  
    }

    updateStatus() ;

}

void Ally::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}

//Ham kiem tra muc tieu con trong tam danh cua Ally khong
bool Ally :: isInRange(Entity* target){

    float dy = target->getPosition().y - this->position.y ; 
    float dx = target->getPosition().x - this->position.x ; 

    
    return (dx * dx + dy * dy) <= ((this->range) * (this->range)) ; 

}

//logic tim kiem va chon quai gan nhat lam muc tieu va danh den khi quai do chet 
void Ally :: updateTarget(const GameContext& context){

    if(target != nullptr && target->isDead()){
        // std :: cout << "tracking dead target, setting target to null" << std :: endl ; 
        this->target = nullptr ; 

    }

    Entity* bestTarget = nullptr ; 
    float minDis = (this->range) * (this->range) ;

    // if(context.enemies.empty())std :: cout << "enemies dang trong !!" << std :: endl ;
    for(auto* entity : context.enemies){

        if(entity == nullptr || entity->isDead()){
            continue; 
        }

        float dis = getDistanceSquared(this->getPosition(), entity->getPosition()); 
        if (dis < 0.0001f) continue;

        if(dis <= minDis){
            minDis = dis ; 
            bestTarget = entity ; 
        }

    }

    target = bestTarget ; 

}

