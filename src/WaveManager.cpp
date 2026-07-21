#include "WaveManager.h"
#include "GameContext.h"
#include "Weapon.h"
#include "Sword.h"
#include <iostream>
#include <cstdlib>

WaveManager::WaveManager()
    : totalMonstersSpawned(0), spawnTimer(0.f), spawnInterval(1.5f), monstersPerWave(3), 
      currentWave(0), waveActive(false), waveDelay(3.f), waveDelayTimer(0.f){ }


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
    // 1. Sinh quái nếu chưa đủ
    if ((int)monsterList.size() < monstersPerWave) {
        spawnTimer += context.deltaTime;
        if (spawnTimer >= spawnInterval) {
            spawnTimer = 0.f;
            float x = 50.f + rand() % 1100;
            float y = 50.f + rand() % 600;
            Monster* m = new Monster(x, y, 100.f, 100.f,50.f, 1.f, 100.f, 10.f) ; 
            Sword* temp = new Sword(20, 100) ; 
            m->setCurrentWeapon(temp) ;
            monsterList.push_back(m);
            std::cout << "Spawned monster! Total: " << monsterList.size() << std::endl;
        }
        return;

    }

    if (monsterList.empty()) {
        waveActive = false;
        currentWave++;
        std::cout << "Wave " << currentWave << " completed!" << std::endl;

        if (currentWave >= maxWaves) {
            gameCompleted = true;
            std::cout << "All waves completed! Game finished!" << std::endl;
        }

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