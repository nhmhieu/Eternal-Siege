#pragma once
#include "Entity.h"

class Ally : public Entity {
private:
	float attackRange;
	float attackCoolDown;
	float cooldownTimer;
public:
	Ally(float x, float y);
	~Ally() override = default;
	
	void update(const GameContext& context) override;
	void draw(sf::RenderWindow& window) override;
};