#include "Ally.h"
#include "BalanceConfig.h"
#include "Bow.h"
#include "EnemyConfig.h"
#include "Map.h"
#include "Player.h"
#include "TextureManager.h"
#include "UpgradeManager.h"
#include "WaveManager.h"

#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace {
enum class ObservedEnemyType {
    Normal,
    Elite,
    Boss
};

struct ExpectedBatch {
    ObservedEnemyType enemyType;
    int count;
};

ObservedEnemyType getEnemyType(const Monster& monster) {
    if (monster.isBoss()) return ObservedEnemyType::Boss;
    if (monster.isElite()) return ObservedEnemyType::Elite;
    return ObservedEnemyType::Normal;
}

std::vector<std::unique_ptr<Monster>> spawnBatch(
    WaveManager& waves,
    const Map& map,
    const ExpectedBatch& expected,
    bool firstBatch
) {
    std::unique_ptr<Monster> first;

    if (firstBatch) {
        // Wave preparation uses deltaTime and stops if update is not called.
        assert(!waves.update(0.4f, map, true));
        assert(!waves.update(0.39f, map, true));
        first = waves.update(0.02f, map, true);
    } else {
        // Confirm the previous batch is clear, then wait for the batch rest.
        assert(!waves.update(0.f, map, true));
        const float restTime =
            expected.enemyType == ObservedEnemyType::Boss ? 2.f : 1.25f;
        assert(!waves.update(restTime - 0.01f, map, true));
        first = waves.update(0.02f, map, true);
    }

    std::vector<std::unique_ptr<Monster>> monsters;
    assert(first);
    assert(getEnemyType(*first) == expected.enemyType);
    monsters.push_back(std::move(first));

    for (int index = 1; index < expected.count; ++index) {
        assert(!waves.update(0.44f, map, false));
        auto next = waves.update(0.02f, map, false);
        assert(next);
        assert(getEnemyType(*next) == expected.enemyType);
        monsters.push_back(std::move(next));
    }

    // No following batch may spawn while one enemy in this batch is alive.
    assert(!waves.update(10.f, map, false));
    return monsters;
}

void verifyEnemyStats(const Monster& monster, int waveNumber) {
    assert(monster.getCurrentWeapon());

    if (monster.isBoss()) {
        const EnemyConfig::Stats stats =
            EnemyConfig::bossStats(waveNumber);
        assert(monster.getMaxHealth() == stats.maxHealth);
        assert(monster.getHealth() == stats.maxHealth);
        assert(monster.getCurrentWeapon()->getDamage() ==
               stats.effectiveDamage);
    } else if (monster.isElite()) {
        const EnemyConfig::Stats boss =
            EnemyConfig::bossStats(waveNumber);
        assert(monster.getMaxHealth() == boss.maxHealth * 0.5f);
        assert(monster.getHealth() == monster.getMaxHealth());
        assert(monster.getCurrentWeapon()->getDamage() ==
               static_cast<int>(
                   static_cast<float>(boss.effectiveDamage) * 0.5f
               ));
        assert(monster.getCollisionBox().size.x > 32.f);
    }
}
}

int main() {
    Map map(15, 15);
    const auto& spawnCells = map.getEnemySpawnCells();
    assert(spawnCells.size() == 4);

    const sf::Vector2i center =
        map.nearestWalkable({map.getWidth() / 2, map.getHeight() / 2});
    for (const sf::Vector2i spawn : spawnCells) {
        assert(map.isWalkable(spawn));
        assert(!map.findPathBFS(spawn, center).empty());
    }

    const std::array<std::vector<ExpectedBatch>, 4> expectedPlans{{
        {{ObservedEnemyType::Normal, 1}},
        {
            {ObservedEnemyType::Normal, 2},
            {ObservedEnemyType::Normal, 2}
        },
        {
            {ObservedEnemyType::Normal, 2},
            {ObservedEnemyType::Normal, 2},
            {ObservedEnemyType::Elite, 2}
        },
        {
            {ObservedEnemyType::Normal, 2},
            {ObservedEnemyType::Elite, 2},
            {ObservedEnemyType::Elite, 2},
            {ObservedEnemyType::Boss, 1}
        }
    }};

    WaveManager waves;
    const std::array<int, 4> expectedTotals{{1, 4, 6, 7}};
    waves.startNextWave();
    assert(waves.getCurrentWave() == 1);
    assert(waves.isWaveActive());

    for (int wave = 1; wave <= waves.getMaxWaves(); ++wave) {
        assert(waves.getCurrentWave() == wave);
        assert(waves.isWaveActive());

        int total = 0;
        int eliteCount = 0;
        int bossCount = 0;
        const auto& plan =
            expectedPlans[static_cast<std::size_t>(wave - 1)];

        for (std::size_t batchIndex = 0;
             batchIndex < plan.size();
             ++batchIndex) {
            auto monsters = spawnBatch(
                waves,
                map,
                plan[batchIndex],
                batchIndex == 0
            );

            for (const auto& monster : monsters) {
                verifyEnemyStats(*monster, wave);
                ++total;
                if (monster->isElite()) ++eliteCount;
                if (monster->isBoss()) ++bossCount;
            }
            // Destroying this vector models clearing the current batch.
        }

        assert(total ==
               expectedTotals[static_cast<std::size_t>(wave - 1)]);
        if (wave == 3) assert(eliteCount == 2);
        if (wave == 4) {
            assert(eliteCount == 4);
            assert(bossCount == 1);
        }

        // Clearing the final batch completes the whole wave.
        assert(!waves.update(0.f, map, true));
        if (wave < waves.getMaxWaves()) {
            assert(waves.isIntermission());
            waves.startNextWave();
        }
    }

    assert(waves.isGameCompleted());
    assert(waves.getCurrentWave() == waves.getMaxWaves());
    waves.startNextWave();
    assert(waves.getCurrentWave() == waves.getMaxWaves());
    assert(!waves.update(60.f, map, true));

    TextureManager textures;
    assert(textures.loadTexture("Ash", "assets/images/Ash.png"));

    const std::array<std::string, 4> allyNames{{
        "Damian", "Evangeline", "Junior", "Lucas"
    }};
    for (const std::string& name : allyNames) {
        assert(textures.loadTexture(
            name,
            "assets/images/" + name + ".png"
        ));
    }

    Player player(textures);
    player.setCurrentWeapon(std::make_unique<Bow>(
        static_cast<float>(BalanceConfig::PLAYER_DAMAGE),
        0.55f
    ));
    player.setAttackCooldown(0.55f);
    assert(player.getHealth() == BalanceConfig::PLAYER_MAX_HEALTH);
    assert(player.getMaxHealth() == BalanceConfig::PLAYER_MAX_HEALTH);
    assert(player.getCurrentWeapon()->getDamage() ==
           BalanceConfig::PLAYER_DAMAGE);

    std::vector<std::unique_ptr<Ally>> allies;
    for (std::size_t index = 0; index < allyNames.size(); ++index) {
        auto ally = std::make_unique<Ally>(
            100.f + static_cast<float>(index) * 50.f,
            100.f,
            textures,
            allyNames[index]
        );
        ally->setCurrentWeapon(std::make_unique<Bow>(
            static_cast<float>(BalanceConfig::ALLY_DAMAGE),
            2.f
        ));
        assert(ally->getHealth() == BalanceConfig::ALLY_MAX_HEALTH);
        assert(ally->getMaxHealth() == BalanceConfig::ALLY_MAX_HEALTH);
        assert(ally->getCurrentWeapon()->getDamage() ==
               BalanceConfig::ALLY_DAMAGE);
        allies.push_back(std::move(ally));
    }

    UpgradeManager upgrades;
    const int initialGold = upgrades.getGold();

    assert(upgrades.purchase(UpgradeType::Damage, player, allies));
    assert(player.getCurrentWeapon()->getDamage() ==
           BalanceConfig::PLAYER_DAMAGE + 4);
    for (const auto& ally : allies) {
        assert(ally->getCurrentWeapon()->getDamage() ==
               BalanceConfig::ALLY_DAMAGE + 3);
    }
    assert(upgrades.undoLastPurchase(player, allies));
    assert(player.getCurrentWeapon()->getDamage() ==
           BalanceConfig::PLAYER_DAMAGE);
    for (const auto& ally : allies) {
        assert(ally->getCurrentWeapon()->getDamage() ==
               BalanceConfig::ALLY_DAMAGE);
    }
    assert(upgrades.getGold() == initialGold);

    const float playerHealth = player.getHealth();
    const float playerMaxHealth = player.getMaxHealth();
    assert(upgrades.purchase(UpgradeType::Vitality, player, allies));
    assert(upgrades.undoLastPurchase(player, allies));
    assert(player.getHealth() == playerHealth);
    assert(player.getMaxHealth() == playerMaxHealth);

    const float playerCooldown = player.getAttackCooldown();
    assert(upgrades.purchase(UpgradeType::FireRate, player, allies));
    assert(upgrades.undoLastPurchase(player, allies));
    assert(std::abs(player.getAttackCooldown() - playerCooldown) < 0.0001f);

    assert(!upgrades.undoLastPurchase(player, allies));
    assert(upgrades.purchase(UpgradeType::Damage, player, allies));
    upgrades.clearUndoHistory();
    assert(!upgrades.undoLastPurchase(player, allies));

    // Recreating gameplay actors applies the balance constants once, rather
    // than multiplying already-buffed values again.
    Player restartedPlayer(textures);
    restartedPlayer.setCurrentWeapon(std::make_unique<Bow>(
        static_cast<float>(BalanceConfig::PLAYER_DAMAGE),
        0.55f
    ));
    assert(restartedPlayer.getMaxHealth() ==
           BalanceConfig::PLAYER_MAX_HEALTH);
    assert(restartedPlayer.getCurrentWeapon()->getDamage() ==
           BalanceConfig::PLAYER_DAMAGE);

    Ally restartedAlly(100.f, 100.f, textures, "Damian");
    restartedAlly.setCurrentWeapon(std::make_unique<Bow>(
        static_cast<float>(BalanceConfig::ALLY_DAMAGE),
        2.f
    ));
    assert(restartedAlly.getMaxHealth() ==
           BalanceConfig::ALLY_MAX_HEALTH);
    assert(restartedAlly.getCurrentWeapon()->getDamage() ==
           BalanceConfig::ALLY_DAMAGE);
    return 0;
}
