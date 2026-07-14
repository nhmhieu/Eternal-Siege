#include <SFML/Graphics.hpp>

#include "Entity.h" 
#include "GameContext.h"

Entity :: Entity(){
    x = 0 ;
    y = 0 ;
    health = 0 ;
    maxHealth = 0 ; 
}

// Định nghĩa và gán giá trị thông qua Initializer List (dấu hai chấm)
Entity::Entity(float x, float y, float health, float maxHealth)
    : x(x), y(y), health(health), maxHealth(maxHealth) 
{
    // Thân hàm có thể để trống hoàn toàn ở đây!
}

Entity :: Entity(float x, float y, float health, float maxHealth, Team team, Weapon* currentWeapon){
    this->x = x ;
    this->y = y ;
    this->health = health ;
    this->maxHealth = maxHealth ; 
    this->team = team ; 
    this->currentWeapon = currentWeapon ; 
}

void Entity :: takeDamage(float damage){
    health -= damage ;
    if(health <= 0) health = 0 ; 

    std :: cout << "Mau cua quai hien tai : " << this->health << endl ; 
}

bool Entity :: isDead() const{
    return false ; 
}

//Ham tao attackHitBox Entity trong vung nay se nhan sat thuong
sf::FloatRect Entity::getAttackHitbox() const {
    // 1. Lấy vị trí và kích thước thực tế theo chuẩn SFML 3.0
    sf::FloatRect currentBounds = sprite.getGlobalBounds(); 
    
    // 2. Định nghĩa kích thước cho hitbox tấn công bằng sf::Vector2f
    float range = 40.f; 
    sf::Vector2f hitboxSize(40.f, 40.f); 
    
    // 3. Tính toán tâm của thực thể (Sử dụng currentBounds.position và currentBounds.size)
    sf::Vector2f center = currentBounds.position + (currentBounds.size / 2.f);
    
    // 4. Đẩy tâm của hitbox ra phía trước dựa theo hướng 'attackDirection' và khoảng cách 'range'
    sf::Vector2f attackCenter = center + (attackDirection * range);
    
    // 5. Tạo ra FloatRect mới: Nhận vào (Vị trí góc trên bên trái, Kích thước)
    sf::Vector2f attackPosition = attackCenter - (hitboxSize / 2.f);
    sf::FloatRect attackHitbox(attackPosition, hitboxSize);

    return attackHitbox;
}

void Entity :: updateStatus(){
    if(isAttacking){

        if(attackClock.getElapsedTime().asSeconds() > attackDuration){
            isAttacking = false ; 
        }

    }
}