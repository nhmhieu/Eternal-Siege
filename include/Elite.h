#pragma once

#include "EnemyConfig.h"
#include "Monster.h"

class Elite final : public Monster {
private:
    Elite(float x, float y, const EnemyConfig::Stats& stats);

public:
    Elite(float x, float y, int waveNumber);
    bool isElite() const override { return true; }
};
