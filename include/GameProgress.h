#pragma once

#include "LevelDefinition.h"

struct RunResult {
    LevelId levelId = LevelId::RuinedCatacombs;
    int goldEarned = 0;
    int totalGold = 0;
    bool firstClear = false;
};

class GameProgress {
public:
    RunResult grantVictory(LevelId level, int reward) {
        const bool first = level == LevelId::RuinedCatacombs && !ruinedCatacombsCleared;
        totalGold += reward;
        if (level == LevelId::RuinedCatacombs) ruinedCatacombsCleared = true;
        return {level, reward, totalGold, first};
    }

    int totalGold = 0;
    bool ruinedCatacombsCleared = false;
    bool castleGateOpen = false;
};
