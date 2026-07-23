#pragma once
#include "Entity.h"

class Ally : public Entity {
private:
	float attackCoolDown;
	//xoa cooldownTimer
	sf :: Vector2f position ; 


	Entity* target ; 
	float range ; 

public:

	void finishAttack() { isAttacking = false; }

	Ally(float x, float y);
	~Ally() override = default;
	
	void update(const GameContext& context) override;
	void draw(sf::RenderWindow& window) override;

	Entity* getTarget() const{return target ;}
	void updateTarget(const GameContext& context) ; //Ham cap nhat muc tieu giong quai


	bool isInRange(Entity* target) ; 


	bool canAttack(){
        // std :: cout << gap << std :: endl ;
        bool isInRange = (range >= gap) ; 
        bool isReady = !isAttacking && coolDownTimer <= 0 ;

        // if(!isInRange) std :: cout << "Khong trong tam danh" << std :: endl ; 
        // if(!isReady) std :: cout << "Quai chua san sang tan cong " << std :: endl ; 
        
        return isInRange && isReady ; 
    }

};