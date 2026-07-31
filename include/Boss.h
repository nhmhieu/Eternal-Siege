#pragma once

#include "EnemyConfig.h"
#include "Monster.h"

class Boss final : public Monster {
private:
    sf::CircleShape aura;
    bool enraged = false;
    Boss(float x, float y, const EnemyConfig::Stats& stats);

public:
    Boss(float x, float y, int waveNumber);
    bool isBoss() const override { return true; }
    void update(GameContext& context) override;
    void draw(sf::RenderWindow& window) override;
};
