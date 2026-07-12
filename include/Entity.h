#pragma once 

#include <SFML/Graphics.hpp>

class GameContext ; 

class Entity{

    protected : 
        
        float x ;
        float y ;
        float health ;
        float maxHealth ;  
        sf :: CircleShape sprite ; 
        sf :: Vector2f direction = {0.f, 1.f} ;//mac dinh nhin xuong duoi

        //cac attribute de thuc hien tan cong
        sf :: Clock attackClock ; 
        bool isAttacking = false ; 
        float attackDuration = 0.2f ; 
        float attackPower = 10.f ; 

    public : 

        //constructor va destructor
        Entity() ;
        Entity(float x, float y, float health, float maxHealth) ;
        virtual ~Entity() = default ; //ham ao default khong can viet logic ham

        //getter 
        float getX() const {return x ;} 
        float getY() const {return y ; } 
        float getHealth() const {return health ;} ;
        
        //Ham tinh toan va xu li logic game
        virtual void takeDamage(float damage) ; 
        bool isDead() const ; 
        
        
        //ham ve Entity ra man hinh 
        virtual void draw(sf :: RenderWindow& window) = 0 ; //(Ham thuan ao bat buoc cac Entity ton tai phai co cach ve ra man hinh)
        virtual void update(const GameContext& context) = 0 ;  //Ham update vi tri/sprite trong frame hien tai 
        
        
        //Cac ham phuc vu cho viec tan cong quai

        //-----Ham lay vung va cham nhan vat/vung nhan sat thuong dua vao sprite 
        virtual sf :: FloatRect getCollisionBox() const {
            return sprite.getGlobalBounds() ; 
        }
        virtual sf :: FloatRect getHurtBox() const {
            return sprite.getGlobalBounds() ; 
        }
        virtual sf :: FloatRect getAttackHitbox() const ; 
        
        bool getIsAttacking() const {return isAttacking ;} 
        float getAttackPower() const {return attackPower ;} 

        void updateStatus() ; 
    } ;