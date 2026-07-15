#pragma once 

#include <SFML/Graphics.hpp> 
#include <unordered_set> 

class Weapon{

    private : 
    //danh sach doi tuong da bi trung don
    std :: unordered_set<Entity*> hitList ; 

    public : 

    //destructor 
    virtual ~Weapon() = default ;  
    


    //Moi vu khi co cach ve hitbox cua rieng minh
    virtual sf :: FloatRect getHitbox(sf :: Vector2f entityCenter, sf :: Vector2f attackDir) = 0 ; 
    virtual bool isHitting(sf::Vector2f attackerPos, sf::Vector2f attackDir, sf::Vector2f targetPos) = 0;

    // getter/setter 
    virtual void setDamage(int damage) = 0 ; 
    virtual int getDamage() const = 0 ; 
    std :: unordered_set<Entity*> getHitList() const {return this->hitList ; }
    bool isHit(Entity* target){
        return hitList.find(target) != hitList.end() ;
    }
    void addHit(Entity* target){
        hitList.insert(target) ; 
    }
    void clearHitList(){
        hitList.clear() ; 
    }

    virtual void drawDebug(sf::RenderTarget& target, sf::Vector2f entityCenter, sf::Vector2f attackDir) {
        // Tự lấy hitbox của chính mình dựa trên vị trí và hướng của chủ thể
        sf::FloatRect hitbox = this->getHitbox(entityCenter, attackDir);

        sf::RectangleShape debugRect;
        debugRect.setPosition(hitbox.position); // SFML 3
        debugRect.setSize(hitbox.size);         // SFML 3
        
        // Màu đỏ trong suốt để dễ nhìn
        debugRect.setFillColor(sf::Color(255, 0, 0, 50)); // Đỏ nhạt xuyên thấu
        debugRect.setOutlineColor(sf::Color::Red);
        debugRect.setOutlineThickness(1.5f);

        target.draw(debugRect);
    }


} ; 