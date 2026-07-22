#pragma once
#include "Entity.h"
#include "TextureManager.h"

class Ally : public Entity {
private:
	float attackRange = 150.f;
	float attackCoolDown = 1.f;
	float cooldownTimer = 0.f;
public:
	Ally(float x, float y);
	Ally(float x, float y, TextureManager& textureManager, const std::string& textureName = "ally");
	~Ally() override = default;
	
	void update(const GameContext& context) override;
	void draw(sf::RenderWindow& window) override;
};