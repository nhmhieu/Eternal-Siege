#include "WaveManager.h"

#include "Boss.h"
#include "Elite.h"
#include "EnemyConfig.h"
#include "Map.h"
#include "Sword.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>

WaveManager::WaveManager()
    : randomEngine(static_cast<std::mt19937::result_type>(
          std::chrono::steady_clock::now().time_since_epoch().count())) {
    wavePlans[0] = {
        {EnemyType::Normal, 1}
    };
    wavePlans[1] = {
        {EnemyType::Normal, 2},
        {EnemyType::Normal, 2}
    };
    wavePlans[2] = {
        {EnemyType::Normal, 2},
        {EnemyType::Normal, 2},
        {EnemyType::Elite, 2}
    };
    wavePlans[3] = {
        {EnemyType::Normal, 2},
        {EnemyType::Elite, 2},
        {EnemyType::Elite, 2},
        {EnemyType::Boss, 1}
    };
}

const WaveManager::WavePlan& WaveManager::getCurrentWavePlan() const {
    return wavePlans[static_cast<std::size_t>(currentWave - 1)];
}

const WaveManager::BatchSpec& WaveManager::getCurrentBatch() const {
    return getCurrentWavePlan()[currentBatchIndex];
}

float WaveManager::getRestTimeBeforeNextBatch() const {
    const std::size_t nextBatchIndex = currentBatchIndex + 1;
    if (nextBatchIndex < getCurrentWavePlan().size() &&
        getCurrentWavePlan()[nextBatchIndex].enemyType == EnemyType::Boss) {
        return BOSS_PREPARATION_TIME;
    }
    return BATCH_REST_TIME;
}

void WaveManager::resetWaveProgress() {
    currentBatchIndex = 0;
    spawnedInCurrentBatch = 0;
    spawnTimer = 0.f;
    phaseTimer = 0.f;
    batchPhase = BatchPhase::Preparing;
}

void WaveManager::completeCurrentWave() {
    if (currentWave >= maxWaves) {
        state = WaveState::Completed;
        std::cout << "All waves completed!\n";
    } else {
        state = WaveState::Intermission;
        std::cout << "Wave " << currentWave << " completed!\n";
    }
}

std::unique_ptr<Monster> WaveManager::createMonster(
    EnemyType enemyType,
    const Map& map
) {
    const auto& spawnCells = map.getEnemySpawnCells();
    if (spawnCells.empty()) {
        return nullptr;
    }

    std::uniform_int_distribution<std::size_t> spawnDistribution(
        0, spawnCells.size() - 1);
    const sf::Vector2f spawnPosition =
        map.gridToWorld(spawnCells[spawnDistribution(randomEngine)]);

    EnemyConfig::Stats stats{};
    std::unique_ptr<Monster> monster;

    switch (enemyType) {
    case EnemyType::Normal:
        stats = EnemyConfig::normalStats(currentWave);
        monster = std::make_unique<Monster>(
            spawnPosition.x,
            spawnPosition.y,
            stats.maxHealth,
            stats.maxHealth,
            stats.attackRange,
            stats.cooldown,
            stats.speed,
            static_cast<float>(stats.effectiveDamage)
        );
        monster->setGoldReward(8 + currentWave * 3);
        break;

    case EnemyType::Elite:
        stats = EnemyConfig::eliteStats(currentWave);
        monster = std::make_unique<Elite>(
            spawnPosition.x,
            spawnPosition.y,
            currentWave
        );
        monster->setGoldReward(8 + currentWave * 3);
        break;

    case EnemyType::Boss:
        stats = EnemyConfig::bossStats(currentWave);
        monster = std::make_unique<Boss>(
            spawnPosition.x,
            spawnPosition.y,
            currentWave
        );
        break;
    }

    monster->setCurrentWeapon(std::make_unique<Sword>(
        stats.effectiveDamage,
        stats.attackRange
    ));

    const sf::FloatRect spawnBounds = monster->getCollisionBox();
    const sf::Vector2f halfExtents = spawnBounds.size / 2.f;
    if (const auto validPosition =
            map.findNearestValidPosition(spawnPosition, halfExtents)) {
        monster->setPosition(validPosition->x, validPosition->y);
    } else {
        return nullptr;
    }
    return monster;
}

std::unique_ptr<Monster> WaveManager::update(
    float deltaTime,
    const Map& map,
    bool noMonstersAlive
) {
    if (state != WaveState::Active) {
        return nullptr;
    }

    const float elapsed = std::max(0.f, deltaTime);

    if (batchPhase == BatchPhase::WaitingForClear) {
        if (!noMonstersAlive) {
            return nullptr;
        }

        if (currentBatchIndex + 1 >= getCurrentWavePlan().size()) {
            completeCurrentWave();
        } else {
            batchPhase = BatchPhase::Resting;
            phaseTimer = 0.f;
        }
        return nullptr;
    }

    if (batchPhase == BatchPhase::Preparing) {
        phaseTimer += elapsed;
        if (phaseTimer < WAVE_PREPARATION_TIME) {
            return nullptr;
        }
        phaseTimer = 0.f;
        batchPhase = BatchPhase::Spawning;
    } else if (batchPhase == BatchPhase::Resting) {
        phaseTimer += elapsed;
        if (phaseTimer < getRestTimeBeforeNextBatch()) {
            return nullptr;
        }

        ++currentBatchIndex;
        spawnedInCurrentBatch = 0;
        spawnTimer = 0.f;
        phaseTimer = 0.f;
        batchPhase = BatchPhase::Spawning;
    }

    if (batchPhase != BatchPhase::Spawning) {
        return nullptr;
    }

    if (spawnedInCurrentBatch > 0) {
        spawnTimer += elapsed;
        if (spawnTimer < SPAWN_INTERVAL) {
            return nullptr;
        }
    }

    const EnemyType enemyType = getCurrentBatch().enemyType;
    ++spawnedInCurrentBatch;
    spawnTimer = 0.f;

    if (spawnedInCurrentBatch >= getCurrentBatch().count) {
        batchPhase = BatchPhase::WaitingForClear;
    }

    return createMonster(enemyType, map);
}

void WaveManager::startNextWave() {
    if (state != WaveState::Intermission || currentWave >= maxWaves) {
        return;
    }

    ++currentWave;
    resetWaveProgress();
    state = WaveState::Active;
    std::cout << "Wave " << currentWave << " started!\n";
}
