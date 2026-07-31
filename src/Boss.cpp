#include "Boss.h"
#include "GameContext.h"
#include "Weapon.h"

Boss::Boss(float x, float y, int waveNumber)
    : Boss(x, y, EnemyConfig::bossStats(waveNumber)) {
}

Boss::Boss(float x, float y, const EnemyConfig::Stats& stats)
    : Monster(x, y,
              stats.maxHealth,
              stats.maxHealth,
              stats.attackRange,
              stats.cooldown,
              stats.speed,
              static_cast<float>(stats.effectiveDamage)),
      aura(31.f) {
    monsterShape.setSize({58.f, 58.f});
    monsterShape.setOrigin({29.f, 29.f});
    monsterShape.setFillColor(sf::Color(130, 45, 185));

    aura.setOrigin({31.f, 31.f});
    aura.setFillColor(sf::Color(220, 80, 255, 45));
    aura.setOutlineColor(sf::Color(235, 150, 255));
    aura.setOutlineThickness(2.f);
    setGoldReward(150);
}

void Boss::update(GameContext& context) {
    // Phase 2: duoi 50% mau, Boss nhanh va tan cong manh hon.
    if (!enraged && getHealth() <= getMaxHealth() * 0.5f) {
        enraged = true;
        moveSpeed *= 1.35f;
        setAttackPower(getAttackPower() * 1.4f);
        setAttackCooldown(getAttackCooldown() * 0.7f);
        if (getCurrentWeapon()) {
            getCurrentWeapon()->setDamage(
                static_cast<int>(getCurrentWeapon()->getDamage() * 1.4f));
        }
        monsterShape.setFillColor(sf::Color(220, 55, 110));
        aura.setOutlineColor(sf::Color::Red);
    }
    Monster::update(context);
}

void Boss::draw(sf::RenderWindow& window) {
    aura.setPosition(position);
    window.draw(aura);
    Monster::draw(window);
}
