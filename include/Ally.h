#pragma once
#include "Entity.h"

class Ally : public Entity {
private:
	float attackRange;
	float attackCoolDown;
	//xoa cooldownTimer


	sf :: Vector2f position ; 


	Entity* target ; 
	float range ; 

public:

	bool canAttackNow() {
		return (target != nullptr && 
				!isAttacking && 
				attackClock.getElapsedTime().asSeconds() >= attackCoolDown);
	}

	void startAttack() { isAttacking = true; attackClock.restart(); }
	void finishAttack() { isAttacking = false; }

	Ally(float x, float y);
	~Ally() override = default;
	
	void update(const GameContext& context) override;
	void draw(sf::RenderWindow& window) override;

	Entity* getTarget() const{return target ;}
	void updateTarget(const GameContext& context) ; //Ham cap nhat muc tieu giong quai


	bool isInRange(Entity* target) ; 


};