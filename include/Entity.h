#pragma once 

#include <SFML/Graphics.hpp>
#include <iostream>

using namespace std ;


class GameContext ; 

enum class Team{

    Player, 
    Enemy, 
    Neutral 

} ;

class Weapon ; 

class Entity{

    protected : 

        Weapon* currentWeapon = nullptr ; 

        Team team ; 
        float x ;
        float y ;
        float health ; //mai mot de lai la int cho de tinh di
        float maxHealth ;  
        sf :: CircleShape sprite ; 
        sf :: Vector2f direction = {0.f, 1.f} ;//mac dinh nhin xuong duoi
        sf :: Vector2f attackDirection = {0.f, 0.f} ; 

        //cac attribute de thuc hien tan cong
        sf :: Clock attackClock ; 
        bool isAttacking = false ; 
        float attackDuration = 0.2f ; 
        float attackPower = 10.f ; 

    public : 

        //constructor va destructor
        Entity() ;
        //constructor nay ghi tam de test cai constructor trong Monster chay
        Entity(float x,float y,float health,float maxHealth) ; 
        Entity(float x, float y, float health, float maxHealth, Team team, Weapon* currentWeapon) ;
        virtual ~Entity() = default ; //ham ao default khong can viet logic ham

        //getter 
        float getX() const {return x ;} 
        float getY() const {return y ; } 
        float getHealth() const {return health ;} ;
        sf :: Vector2f getAttackDirection() const {return attackDirection ;} 
        Team getTeam() const{return this->team ;} 
        Weapon* getCurrentWeapon() const {return this->currentWeapon;} 


        //setter 
        void setX(float x){this->x = x ;}  
        void setY(float y){this->y = y ;} 
        void setDirection(sf :: Vector2f direction){this->direction = direction ;}
        void setAttackDirection(sf :: Vector2f attackDir){this->attackDirection = attackDir ; 
        cout << "Ham attackSetDirection duoc goi, x = " << attackDirection.x << ", y = " << attackDirection.y  << endl ;}
        // void setIsDead(bool status){this->isDead = status ;} 
        void setCurrentWeapon(Weapon* weapon) {this->currentWeapon = weapon ;}

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

        //Ham ao update muc tieu cho tru va quai
        virtual void updateTarget(const std :: vector<Entity*>& targets) {}

        //Ham nhan sat thuong 
        // virtual void takeDamage(int amount) ; 

    } ;