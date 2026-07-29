#include "WaveManager.h"
#include "GameContext.h"
#include <iostream>
#include <cstdlib>

WaveManager::WaveManager()
    : totalMonstersSpawned(0), spawnTimer(0.f), spawnInterval(0.5f), monstersPerWave(5), 
      currentWave(0), waveActive(false), waveDelay(1.f), waveDelayTimer(0.f){ }


void WaveManager::update(const GameContext& context, std::vector<Monster*>& monsterList) {
    
    if (gameCompleted) return;

    if (!waveActive) {
        waveDelayTimer += context.deltaTime;
        if (waveDelayTimer >= waveDelay) {
            waveDelayTimer = 0.f;
            startWave();
        }
        return;
    }

    // Sinh quái nếu chưa đủ số lượng trong wave
    if (totalMonstersSpawned < monstersPerWave) {
        spawnTimer += context.deltaTime;
        if (spawnTimer >= spawnInterval) {
            spawnTimer = 0.f;
            float x = 50.f + rand() % 1100;
            float y = 50.f + rand() % 600;
            monsterList.push_back(new Monster(x, y, 100.f, 100.f, 50.f, 1.f, 100.f, 10.f));
            totalMonstersSpawned++;
            std::cout << "Spawned monster! Total now: " << monsterList.size() << ", spawned: " << totalMonstersSpawned << std::endl;
        }
    }

    // Kiểm tra nếu đã spawn đủ quái và tất cả đã chết → wave hoàn thành
    if (monsterList.empty() && totalMonstersSpawned >= monstersPerWave) {
        waveActive = false;
        currentWave++;
        std::cout << "Wave " << currentWave << " completed!" << std::endl;

        if (currentWave >= maxWaves) {
            gameCompleted = true;
            std::cout << "All waves completed! Game finished!" << std::endl;
        }
        return;
    }
}

void WaveManager::draw(sf::RenderWindow& window, const std::vector<Monster*>& monsterList) {
    for (auto* monster : monsterList) {
        monster->draw(window);
    }
}

void WaveManager::startWave() {
    waveActive = true;
    spawnTimer = 0.f;
    totalMonstersSpawned = 0;
    std::cout << "Wave" << currentWave + 1 << " started!" << std::endl;
}

bool WaveManager::isWaveActive() const {
    return waveActive;
}
void WaveManager::setSpawnInterval(float interval) {
    spawnInterval = interval;
}

bool WaveManager::isGameCompleted() const {
    return currentWave >= maxWaves;
}