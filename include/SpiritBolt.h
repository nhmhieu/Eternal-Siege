#pragma once

#include "Projectiles.h"

class SpiritBolt final : public Projectiles {
public:
    SpiritBolt(sf::Vector2f startPosition, sf::Vector2f direction,
               float speed, float damage, Team shooterTeam);
};
