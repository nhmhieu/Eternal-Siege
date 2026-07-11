#pragma once 

#include <SFML/Graphics.hpp>

class Entity{

    protected : 
        
        float x ;
        float y ;
        float health ;
        float maxHealth ;  

    public : 
        Entity() ;
        Entity(float x, float y, float health, float maxHealth) ;
        virtual ~Entity() ; 

        //getter 
        float getX() const {return x ;} 
        float getY() const {return y ; } 
        float getHealth() const {return health ;} ;


        virtual void takeDamage(float damage) ; 
        bool isDead() const ; 


        //ham ve Entity ra man hinh 
        virtual void draw(sf :: RenderWindow& window) = 0 ; ///(Ham thuan ao bat buoc cac Entity ton tai phai co cach ve ra man hinh)
         
} ;