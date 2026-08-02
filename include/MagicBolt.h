#pragma once

#include "Projectiles.h"

class MagicBolt final : public Projectiles {
public:
    MagicBolt(sf::Vector2f startPosition,
              sf::Vector2f direction,
              float speed,
              float damage,
              Team shooterTeam);
};
