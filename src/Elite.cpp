#include "Elite.h"

Elite::Elite(float x, float y, int waveNumber)
    : Elite(x, y, EnemyConfig::eliteStats(waveNumber)) {
}

Elite::Elite(float x, float y, const EnemyConfig::Stats& stats)
    : Monster(
          x,
          y,
          stats.maxHealth,
          stats.maxHealth,
          stats.attackRange,
          stats.cooldown,
          stats.speed,
          static_cast<float>(stats.effectiveDamage)
      ) {
    constexpr float eliteSize = 32.f * 1.15f;
    monsterShape.setSize({eliteSize, eliteSize});
    monsterShape.setOrigin({eliteSize * 0.5f, eliteSize * 0.5f});
    monsterShape.setFillColor(sf::Color(235, 145, 45));
    presentationTint = sf::Color(235, 145, 45);
    monsterShape.setOutlineColor(sf::Color(255, 220, 105));
    monsterShape.setOutlineThickness(2.f);
    collisionSize = {eliteSize, eliteSize};
    hurtBoxSize = {eliteSize, eliteSize};
    visualSize = {eliteSize, eliteSize};
    eliteVisual = true;
}
