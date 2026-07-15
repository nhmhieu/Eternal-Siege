#pragma once
#include "Monster.h"
#include <vector>

class GameContext;

class WaveManager {
private:
	float spawnTimer;
	float spawnInterval;
	int monstersPerWave;
	int currentWave;
	bool waveActive;
	float waveDelay;
	float waveDelayTimer;

public:
	WaveManager();
	void update(const GameContext& context, std::vector<Monster*>& monsterList);
	void draw(sf::RenderWindow& window, const std::vector<Monster*>& monsterList);
	void startWave();
	bool isWaveActive() const;
	void setSpawnInterval(float interval);
};