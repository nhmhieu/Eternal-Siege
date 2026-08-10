#pragma once

#include "Monster.h"

#include <array>
#include <cstddef>
#include <memory>
#include <random>
#include <vector>

class Map;

class WaveManager {
private:
    enum class WaveState {
        Active,
        Intermission,
        Completed
    };

    enum class EnemyType {
        Normal,
        Elite,
        Boss
    };

    struct BatchSpec {
        EnemyType enemyType;
        int count;
    };

    using WavePlan = std::vector<BatchSpec>;

    enum class BatchPhase {
        Preparing,
        Spawning,
        WaitingForClear,
        Resting
    };

    static constexpr float WAVE_PREPARATION_TIME = 0.8f;
    static constexpr float SPAWN_INTERVAL = 0.45f;
    static constexpr float BATCH_REST_TIME = 1.25f;
    static constexpr float BOSS_PREPARATION_TIME = 2.f;

    std::array<WavePlan, 4> wavePlans;
    int currentWave = 1;
    int maxWaves = 4;
    WaveState state = WaveState::Active;

    std::size_t currentBatchIndex = 0;
    int spawnedInCurrentBatch = 0;
    float spawnTimer = 0.f;
    float phaseTimer = 0.f;
    BatchPhase batchPhase = BatchPhase::Preparing;

    std::mt19937 randomEngine;

    const WavePlan& getCurrentWavePlan() const;
    const BatchSpec& getCurrentBatch() const;
    float getRestTimeBeforeNextBatch() const;
    void resetWaveProgress();
    void completeCurrentWave();
    std::unique_ptr<Monster> createMonster(
        EnemyType enemyType,
        const Map& map
    );

public:
    WaveManager();

    std::unique_ptr<Monster> update(
        float deltaTime,
        const Map& map,
        bool noMonstersAlive
    );

    void startNextWave();

    bool isWaveActive() const { return state == WaveState::Active; }
    bool isGameCompleted() const { return state == WaveState::Completed; }
    int getCurrentWave() const { return currentWave; }
    int getMaxWaves() const { return maxWaves; }
    bool isIntermission() const {
        return state == WaveState::Intermission;
    }
};
