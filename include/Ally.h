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
	
	void update(GameContext& context) override;
	void draw(sf::RenderWindow& window) override;

	Entity* getTarget() const{return target ;}
	void updateTarget(const GameContext& context) ; //Ham cap nhat muc tieu giong quai


	bool isInRange(Entity* target) ; 


	bool canAttack(){
        // std :: cout << gap << std :: endl ;

        bool isReady = !isAttacking && coolDownTimer <= 0 ;
		// if(!isReady) std :: cout << "chua san sang tan cong !!" << std :: endl ;

        // if(!isInRange) std :: cout << "Khong trong tam danh" << std :: endl ; 
        // if(!isReady) std :: cout << "Quai chua san sang tan cong " << std :: endl ; 

		if(!isInRange(target)){
			// std :: cout << "Khong trong tam danh !!  : " << this->range << std :: endl ;

		}

        if(target){
			return isInRange(target) && isReady ; 

		}
		return false ; 
    }

};