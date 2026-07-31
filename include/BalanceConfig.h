#pragma once

namespace BalanceConfig {
    constexpr float PLAYER_BASE_MAX_HEALTH = 180.f;
    constexpr float PLAYER_HEALTH_MULTIPLIER = 1.5f;
    constexpr float PLAYER_MAX_HEALTH =
        PLAYER_BASE_MAX_HEALTH * PLAYER_HEALTH_MULTIPLIER;

    constexpr int PLAYER_BASE_DAMAGE = 12;
    constexpr float PLAYER_DAMAGE_MULTIPLIER = 1.5f;
    constexpr int PLAYER_DAMAGE = static_cast<int>(
        PLAYER_BASE_DAMAGE * PLAYER_DAMAGE_MULTIPLIER + 0.5f);

    constexpr float ALLY_BASE_MAX_HEALTH = 180.f;
    constexpr float ALLY_HEALTH_MULTIPLIER = 2.f;
    constexpr float ALLY_MAX_HEALTH =
        ALLY_BASE_MAX_HEALTH * ALLY_HEALTH_MULTIPLIER;

    constexpr int ALLY_BASE_DAMAGE = 18;
    constexpr float ALLY_DAMAGE_MULTIPLIER = 1.25f;
    constexpr int ALLY_DAMAGE = static_cast<int>(
        ALLY_BASE_DAMAGE * ALLY_DAMAGE_MULTIPLIER + 0.5f);
}
