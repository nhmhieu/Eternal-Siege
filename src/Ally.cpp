#include "Ally.h"
#include "GameContext.h"
#include "MathUtils.h"

#include <cmath>
#include <iostream>

Ally::Ally(float x, float y)
    : Entity(x, y, 50.f, 50.f), range(100.f), attackCoolDown(1.f)
{
    sprite.setRadius(20.f);
    sprite.setFillColor(sf::Color::Blue);
    sprite.setOrigin(sf::Vector2f(20.f, 20.f));
    sprite.setPosition(sf::Vector2f(x, y));
    team = Team::Player;
    
    this->target = nullptr ; //Muc tieu ban dau la null
    isAttacking = false ;
    range = 100.f ; 
}

void Ally::update(const GameContext& context) {
    //cap nhat target gan nhat hien co trong context
    updateTarget(context); 

    //neu nhu co target va isAttacking dang bi tat
    if(target && !isAttacking){

        //nap huong tan cong moi nhat va truyen vao attribute 

        // SỬA LỖI BIẾN: Đã đổi tất cả thành 'dir' cho đồng nhất
        sf::Vector2f dir = target->getPosition() - this->getPosition(); 
        
        float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        
        if (length != 0.f) {
            dir /= length;
        } else {
            dir = { 1.f, 0.f }; 
        }
        
        this->setAttackDirection(dir);
        
        // 3. Đủ điều kiện mới bật lên TRUE và reset đồng hồ
        isAttacking = true; 
        //bat dau dem attackClock
        // attackClock.restart(); 
    }
    // Không cần hàm else { isAttacking = false; } ở đây nữa!

    if(isDying){
        updateDeadTimer(context) ; 
    }

    if(isAttacking){
        updateAttackTimer(context) ; 
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

    return dx * dx + dy * dy <= (this->range) * (this->range) ; 


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

        if(entity == nullptr && entity->isDead()){
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

