#include "Monster.h" 
#include <cmath>
#include <iostream>
#include <limits>

/// constructor mac dinh
Monster::Monster() : Monster(0, 0, 10, 10) {
    // Không cần làm gì thêm ở đây
}

// Constructor co tham so
Monster::Monster(float x, float y, float health, float maxHealth) 
    : Entity(x, y, health, maxHealth), attackRange(50.0f), speed(100.0f), currentTarget(nullptr), targetTimer(0.0f) {

}

void Monster :: updateTarget(const std :: vector<Entity*>& allTargets){
    Entity* closest = nullptr ; 
    float minDistanceSquared = std::numeric_limits<float>::max(); // Giá trị lớn nhất có thể


    for(Entity* target : allTargets){
        if(target == nullptr || target->isDead()) continue; //bo qua nhan vat
        
        float dx = target->getX() - this->getX() ;
        float dy = target->getY() - this->getY() ;
        
        float distanceSq = dx*dx +  dy*dy ;
        
        if(distanceSq < minDistanceSquared){
            minDistanceSquared = distanceSq ;
            closest = target ; 
        }
    }


    this->currentTarget = closest ; 

}

void Monster :: moveToward(float deltaTime, Entity* target){

    if(!target) return ; //neu khong co muc tieu thi quai dung yen

    float dx = target->getX() - this->x ; 
    float dy = target->getY() - this->y ; 

    float distanceSquared = dx * dx + dy * dy ;
    float distance = sqrt(distanceSquared) ; 



    if(distance <= attackRange){
        std :: cout << "Quai dang tan cong !" << std :: endl ; 

    }

    else{
        float moveX = (dx / distance )  * speed * deltaTime ; 
        float moveY = (dy / distance ) * speed * deltaTime ; 

        if (distance > 0.0001f) {
            this->x += moveX ; 
            this->y += moveY ;
        }
    }   
}

void Monster :: update(float deltaTime, const std :: vector<Entity*>& allTargets) {

    targetTimer += deltaTime ; 
    if(targetTimer >= 0.5f){
        updateTarget(allTargets) ;
        targetTimer = 0 ; 
    }

    if(currentTarget != nullptr){
        
        if(currentTarget->isDead()){
            currentTarget = nullptr ;
            return ; 
        }

        moveToward(deltaTime, currentTarget) ; 
    }
}