#include "Monster.h" 
#include "GameContext.h"

#include <cmath>
#include <iostream>
#include <limits>

/// constructor mac dinh
Monster::Monster() : Monster(0, 0, 10, 10) {
    // Không cần làm gì thêm ở đây

}

// Constructor co tham so
Monster::Monster(float x, float y, float health, float maxHealth) 
    : Entity(x, y, health, maxHealth), 
      attackRange(50.0f), 
      speed(100.0f), 
      currentTarget(nullptr), 
      targetTimer(0.0f) 
{
    // 1. Thiết lập hình dáng cho quái vật
    // Thay vì gọi là sprite, chúng ta dùng body (sf::CircleShape)
    sprite.setRadius(20.0f);
    sprite.setFillColor(sf::Color::Red);
    
    // 2. Thiết lập tâm để dễ dàng định vị
    sprite.setOrigin(sf::Vector2f(20.0f, 20.0f)); 
    
    // 3. Đặt vị trí ban đầu
    sprite.setPosition(sf::Vector2f(x, y));
}

//Ham huy va giai phong con tro currentTarget
Monster :: ~Monster(){
    // delete currentTarget ; 
}

//Cac ham tim target gan nhat cho quai vat di chuyen tien den target
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

    // --- IN RA ĐỂ KIỂM TRA QUÁI CÓ NHẬN DIỆN ĐƯỢC KHOẢNG CÁCH KHÔNG ---
    // std::cout << "Quai dang o: " << this->x << "," << this->y 
    //           << " | Target o: " << target->getX() << "," << target->getY() 
    //           << " | Distance: " << distance << std::endl;

    // if(distance <= attackRange){
    //     std :: cout << "Quai dang tan cong !" << std :: endl ; 
    // }



    if(distance <= attackRange){
        // std :: cout << "Quai dang tan cong !" << std :: endl ; 

    }

    else{
        //Ham xu li de quai khong di chuyen de len player
        
        if (distance > 0.0001f) {

            float moveX = (dx / distance )  * speed * deltaTime ; 
            float moveY = (dy / distance ) * speed * deltaTime ; 
            this->x += moveX ; 
            this->y += moveY ;
        }
        this->sprite.setPosition(sf::Vector2f(this->x, this->y));
    }   

}

void Monster :: update(const GameContext& context) {

    targetTimer += context.deltaTime ; 
    if(targetTimer >= 0.5f){
        std::cout << "Quai dang quet tim muc tieu... So luong Entity nhan duoc: " << context.allEntity.size() << std::endl;

        //quai chi nhin vao vector chua player de tan cong, va lien tuc tim muc tieu gan nhat
        updateTarget(context.players) ;
        targetTimer = 0 ; 
    }

    if(currentTarget != nullptr){
        
        if(currentTarget->isDead()){
            currentTarget = nullptr ;
            return ; 
        }

        moveToward(context.deltaTime, currentTarget) ; 
    } 
    else{
        std::cout << "Quai khong co mục tieu (currentTarget == nullptr)!" << std::endl;
    

    }

    this->sprite.setPosition(sf::Vector2f(this->x, this->y));

}

void Monster :: draw(sf :: RenderWindow& window){
    window.draw(this->sprite) ; 
}