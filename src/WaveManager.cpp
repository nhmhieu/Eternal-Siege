#include "WaveManager.h"
#include "GameContext.h"
#include "Weapon.h"
#include "Sword.h"
#include <iostream>
#include <cstdlib>
#include <memory>   // Cho std::make_unique
#include "Constants.h"

WaveManager::WaveManager()
    : totalMonstersSpawned(0), spawnTimer(0.f), spawnInterval(0.5f), monstersPerWave(5),
    currentWave(0), waveActive(false), waveDelay(1.f), waveDelayTimer(0.f) {
}

void WaveManager::update(const GameContext& context, std::vector<Monster*>& monsterList) {
    if (gameCompleted) return;

    // 1. Đợi giữa các wave
    if (!waveActive) {
        waveDelayTimer += context.deltaTime;
        if (waveDelayTimer >= waveDelay) {
            waveDelayTimer = 0.f;
            startWave();
        }
        return;
    }

    // 2. Sinh quái nếu chưa đủ số lượng
    if (totalMonstersSpawned < monstersPerWave) {
        spawnTimer += context.deltaTime;
        if (spawnTimer >= spawnInterval) {
            spawnTimer = 0.f;
            const int mapWidth = static_cast<int>(
                GameConfig::DEFAULT_MAP_WIDTH * GameConfig::TILE_SIZE);
            const int mapHeight = static_cast<int>(
                GameConfig::DEFAULT_MAP_HEIGHT * GameConfig::TILE_SIZE);
            constexpr int margin = 20;

            float x = 0.f;
            float y = 0.f;
            switch (rand() % 4) {
            case 0:
                x = static_cast<float>(margin);
                y = static_cast<float>(margin + rand() % (mapHeight - 2 * margin));
                break;
            case 1:
                x = static_cast<float>(mapWidth - margin);
                y = static_cast<float>(margin + rand() % (mapHeight - 2 * margin));
                break;
            case 2:
                x = static_cast<float>(margin + rand() % (mapWidth - 2 * margin));
                y = static_cast<float>(margin);
                break;
            default:
                x = static_cast<float>(margin + rand() % (mapWidth - 2 * margin));
                y = static_cast<float>(mapHeight - margin);
                break;
            }

            // Tạo Monster và gán vũ khí (dùng unique_ptr)
            Monster* m = new Monster(x, y, 100.f, 100.f, 50.f, 1.f, 100.f, 10.f);
            m->setCurrentWeapon(std::make_unique<Sword>(10, 50));
            monsterList.push_back(m);
            totalMonstersSpawned++;   //biến đếm

            std::cout << "Spawned monster! Total: " << monsterList.size()
                << ", spawned this wave: " << totalMonstersSpawned << std::endl;
        }
        return;   // Đang sinh, chưa kết thúc wave
    }

    // 3. Kiểm tra hoàn thành wave (khi đã spawn đủ và tất cả quái chết)
    if (monsterList.empty() && totalMonstersSpawned >= monstersPerWave) {
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
    std::cout << "Wave " << currentWave + 1 << " started!" << std::endl;
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
