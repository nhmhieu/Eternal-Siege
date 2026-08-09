#pragma once
#include "Projectiles.h"

class HeavySpiritBolt final : public Projectiles {
public:
    HeavySpiritBolt(sf::Vector2f start, sf::Vector2f direction, float damage,
                    Team team, float splashDamage, float splashRadius);
};
