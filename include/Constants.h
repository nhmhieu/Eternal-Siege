#pragma once

namespace GameConfig {
    // Gameplay actions are handled from KeyPressed edges. OS key-repeat would
    // otherwise toggle pause or purchase/undo upgrades more than once.
    constexpr bool KEY_REPEAT_ENABLED = false;

    constexpr float TILE_SIZE = 48.f;

    constexpr int DEFAULT_MAP_WIDTH = 15;
    constexpr int DEFAULT_MAP_HEIGHT = 15;

    // Map chiếm từ x = 0 đến x = 720.
    constexpr float HUD_LEFT =
        static_cast<float>(DEFAULT_MAP_WIDTH) * TILE_SIZE;

    constexpr float WINDOW_WIDTH = 1280.f;
    constexpr float WINDOW_HEIGHT = 720.f;
}
