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
            108.f + static_cast<float>(waveNumber) * 30.f,
            50.f,
            std::max(
                0.7f,
                1.3f - static_cast<float>(waveNumber) * 0.08f
            ),
            72.f + static_cast<float>(waveNumber) * 5.f,
            8 + static_cast<int>(static_cast<float>(waveNumber) * 2.2f)
        };
    }

    inline Stats bossStats(int waveNumber) {
        return {
            830.f + static_cast<float>(waveNumber) * 95.f,
            62.f,
            0.8f,
            62.f,
            30
        };
    }

    inline Stats eliteStats(int waveNumber) {
        const Stats boss = bossStats(waveNumber);
        return {
            boss.maxHealth * 0.55f,
            boss.attackRange,
            boss.cooldown,
            boss.speed * 1.07f,
            static_cast<int>(
                static_cast<float>(boss.effectiveDamage) * 0.58f
            )
        };
    }
}
