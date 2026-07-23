#pragma once
#include "Monster.h"
#include <vector>

class GameContext;

class WaveManager {
private:
	int totalMonstersSpawned;
	float spawnTimer;
	float spawnInterval;
	int monstersPerWave;
	int currentWave;
	bool waveActive;
	float waveDelay;
	float waveDelayTimer;
	int maxWaves = 4;
	bool gameCompleted = false;

public:
	WaveManager();
	void update(const GameContext& context, std::vector<Monster*>& monsterList);
	void draw(sf::RenderWindow& window, const std::vector<Monster*>& monsterList);
	void startWave();
	bool isWaveActive() const;
	void setSpawnInterval(float interval);
	bool isGameCompleted() const;
};