#pragma once 

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
} ;