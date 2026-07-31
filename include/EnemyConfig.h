#pragma once

#include <algorithm>

namespace EnemyConfig {
    struct Stats {
        float maxHealth;
        float attackRange;
        float cooldown;
        float speed;
        int effectiveDamage;
    };

    inline Stats normalStats(int waveNumber) {
        return {
            85.f + static_cast<float>(waveNumber) * 25.f,
            50.f,
            std::max(
                0.7f,
                1.3f - static_cast<float>(waveNumber) * 0.08f
            ),
            72.f + static_cast<float>(waveNumber) * 5.f,
            7 + waveNumber * 2
        };
    }

    inline Stats bossStats(int waveNumber) {
        return {
            700.f + static_cast<float>(waveNumber) * 80.f,
            62.f,
            0.8f,
            62.f,
            28
        };
    }

    inline Stats eliteStats(int waveNumber) {
        const Stats boss = bossStats(waveNumber);
        return {
            boss.maxHealth * 0.5f,
            boss.attackRange,
            boss.cooldown,
            boss.speed,
            static_cast<int>(
                static_cast<float>(boss.effectiveDamage) * 0.5f
            )
        };
    }
}
