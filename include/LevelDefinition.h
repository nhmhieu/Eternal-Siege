#pragma once
#include <array>
#include <string_view>

enum class LevelId { RuinedCatacombs, SealedDepths, ForgottenThrone };

struct LevelDefinition {
    LevelId id;
    std::string_view title;
    std::string_view description;
    std::string_view objective;
    int difficulty;
    bool unlocked;
    int clearReward;
};

inline constexpr std::array<LevelDefinition, 3> LEVEL_DEFINITIONS{{
    {LevelId::RuinedCatacombs, "Dungeon 01 - Ruined Catacombs",
     "The first breach beneath the Eternal Kingdom.",
     "Survive four waves and defeat the final Boss", 1, true, 300},
    {LevelId::SealedDepths, "Dungeon 02 - Sealed Depths",
     "COMING SOON", "LOCKED", 2, false, 0},
    {LevelId::ForgottenThrone, "Dungeon 03 - Forgotten Throne",
     "COMING SOON", "LOCKED", 3, false, 0}
}};
