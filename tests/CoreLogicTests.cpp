#include "Ally.h"
#include "Arrow.h"
#include "BalanceConfig.h"
#include "Bow.h"
#include "Boss.h"
#include "CombatManager.h"
#include "EnemyConfig.h"
#include "Elite.h"
#include "EntityCollision.h"
#include "EntityLifecycle.h"
#include "GameContext.h"
#include "Map.h"
#include "Player.h"
#include "Sword.h"
#include "TextureManager.h"
#include "UpgradeManager.h"
#include "WaveManager.h"

#include <array>
#include <algorithm>
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

bool containsPointer(
    const std::vector<Entity*>& pointers,
    const Entity* expected
) {
    return std::find(pointers.begin(), pointers.end(), expected) !=
           pointers.end();
}

void assertFinite(sf::Vector2f value) {
    assert(std::isfinite(value.x));
    assert(std::isfinite(value.y));
}

void verifyMapCollisionRules(Map& map) {
    const sf::Vector2f grass = map.gridToWorld({6, 6});
    const sf::Vector2f path = map.gridToWorld({7, map.getHeight() / 2});
    const sf::Vector2f spawn = map.gridToWorld(
        map.getEnemySpawnCells().front());
    assert(!map.collidesWithSolid({grass - sf::Vector2f(5.f, 5.f),
                                   {10.f, 10.f}}));
    assert(!map.collidesWithSolid({path - sf::Vector2f(5.f, 5.f),
                                   {10.f, 10.f}}));
    assert(!map.collidesWithSolid({spawn - sf::Vector2f(5.f, 5.f),
                                   {10.f, 10.f}}));

    const sf::Vector2f wall = map.gridToWorld({0, 0});
    assert(map.collidesWithSolid({wall - sf::Vector2f(5.f, 5.f),
                                  {10.f, 10.f}}));
    assert(map.collidesWithSolid({{-1.f, 100.f}, {10.f, 10.f}}));
    assert(map.collidesWithSolid({
        {map.getWorldBounds().size.x - 5.f, 100.f},
        {10.f, 10.f}
    }));
}

void verifyMovementCollision(
    Map& map,
    TextureManager& textures
) {
    Player player(textures);

    player.setPosition({150.f, 160.f});
    player.moveWithCollision({200.f, 0.f}, map);
    assert(player.getPosition().x <= 174.01f);
    assert(!map.collidesWithSolid(player.getCollisionBox()));

    player.setPosition({150.f, 160.f});
    player.moveWithCollision({200.f, 30.f}, map);
    assert(player.getPosition().x <= 174.01f);
    assert(player.getPosition().y > 160.f);
    assert(!map.collidesWithSolid(player.getCollisionBox()));

    player.setPosition({150.f, 160.f});
    player.moveWithCollision({5000.f, 0.f}, map);
    assert(player.getPosition().x <= 174.01f);
    assert(!map.collidesWithSolid(player.getCollisionBox()));

    Monster normal(150.f, 160.f);
    Elite elite(150.f, 160.f, 3);
    Boss boss(150.f, 160.f, 4);
    std::array<Monster*, 3> variants{{&normal, &elite, &boss}};
    for (Monster* variant : variants) {
        variant->moveWithCollision({500.f, 0.f}, map);
        assert(!map.collidesWithSolid(variant->getCollisionBox()));
        assert(variant->getPosition().x < 192.f);
    }

    player.setPosition({264.f, 160.f});
    GameContext chaseContext;
    CombatManager chaseCombat;
    chaseContext.map = &map;
    chaseContext.combatManager = &chaseCombat;
    chaseContext.deltaTime = 0.2f;
    chaseContext.players = {&player};
    for (Monster* variant : variants) {
        variant->setPosition(150.f, 160.f);
        variant->setCurrentWeapon(std::make_unique<Sword>(1, 50.f));
        for (int frame = 0; frame < 60; ++frame) {
            variant->update(chaseContext);
            assert(!map.collidesWithSolid(variant->getCollisionBox()));
        }
    }
}

void verifySeparation(
    Map& map,
    TextureManager& textures
) {
    const sf::Vector2f openPosition = map.gridToWorld({7, 7});
    Monster first(openPosition.x, openPosition.y);
    Monster second(openPosition.x, openPosition.y);
    std::vector<Monster*> pair{&first, &second};
    std::vector<Ally*> noAllies;
    EntityCollision::separateLivingEntities(map, pair, noAllies);
    assertFinite(first.getPosition());
    assertFinite(second.getPosition());
    assert(first.getPosition() != second.getPosition());
    assert(!first.getCollisionBox()
                .findIntersection(second.getCollisionBox())
                .has_value());

    Ally fixed(
        openPosition.x, openPosition.y, textures, "Damian");
    Monster approaching(openPosition.x, openPosition.y);
    const sf::Vector2f allyBefore = fixed.getPosition();
    std::vector<Monster*> oneMonster{&approaching};
    std::vector<Ally*> oneAlly{&fixed};
    EntityCollision::separateLivingEntities(
        map, oneMonster, oneAlly);
    assert(fixed.getPosition() == allyBefore);
    assertFinite(approaching.getPosition());
    assert(!approaching.getCollisionBox()
                .findIntersection(fixed.getCollisionBox())
                .has_value());

    // Correction next to a wall must move away from the wall, not into it.
    Ally wallAlly(170.f, 160.f, textures, "Damian");
    Monster wallMonster(169.f, 160.f);
    std::vector<Monster*> nearWallMonsters{&wallMonster};
    std::vector<Ally*> nearWallAllies{&wallAlly};
    EntityCollision::separateLivingEntities(
        map, nearWallMonsters, nearWallAllies);
    assert(!map.collidesWithSolid(wallMonster.getCollisionBox()));
    assert(!map.collidesWithSolid(wallAlly.getCollisionBox()));
}

void verifyTargetAndCacheInvalidation(
    Map& map,
    TextureManager& textures
) {
    CombatManager combat;
    GameContext context;
    context.map = &map;
    context.combatManager = &combat;

    Ally attacker(300.f, 300.f, textures, "Damian");
    attacker.setCurrentWeapon(std::make_unique<Sword>(1000, 105.f));
    Monster doomed(340.f, 300.f);
    Monster replacement(360.f, 300.f);

    context.allEntity = {&attacker, &doomed, &replacement};
    context.players = {&attacker};
    context.allies = {&attacker};
    context.enemies = {&doomed};
    context.monsters = {&doomed};
    attacker.updateTarget(context);
    assert(attacker.getTarget() == &doomed);

    attacker.setAttackDirection({1.f, 0.f});
    attacker.setIsAttacking(true);
    combat.processAttack(
        &attacker, attacker.getCurrentWeapon(), context.enemies);
    assert(doomed.isDead());
    assert(attacker.getCurrentWeapon()->isHit(&doomed));
    assert(combat.hasHit(&doomed));

    EntityLifecycle::invalidateReferencesTo(
        {&doomed}, context, combat, false);
    assert(attacker.getTarget() == nullptr);
    assert(!attacker.getCurrentWeapon()->isHit(&doomed));
    assert(!combat.hasHit(&doomed));
    assert(!containsPointer(context.enemies, &doomed));
    assert(containsPointer(context.allEntity, &doomed));

    context.enemies.push_back(&replacement);
    context.monsters.push_back(&replacement);
    attacker.updateTarget(context);
    assert(attacker.getTarget() == &replacement);

    EntityLifecycle::invalidateReferencesTo(
        {&doomed}, context, combat, true);
    assert(!containsPointer(context.allEntity, &doomed));

    Ally firstTarget(300.f, 340.f, textures, "Junior");
    Ally secondTarget(350.f, 340.f, textures, "Lucas");
    Monster hunter(320.f, 340.f);
    context.allEntity = {&hunter, &firstTarget, &secondTarget};
    context.players = {&firstTarget};
    context.allies = {&firstTarget};
    context.enemies = {&hunter};
    context.monsters = {&hunter};

    hunter.updateTarget(context.players);
    assert(hunter.getCurrentTarget() == &firstTarget);
    firstTarget.takeDamage(firstTarget.getMaxHealth());
    EntityLifecycle::invalidateReferencesTo(
        {&firstTarget}, context, combat, false);
    assert(hunter.getCurrentTarget() == nullptr);

    context.players.push_back(&secondTarget);
    context.allies.push_back(&secondTarget);
    hunter.updateTarget(context.players);
    assert(hunter.getCurrentTarget() == &secondTarget);
    secondTarget.takeDamage(secondTarget.getMaxHealth());
    EntityLifecycle::invalidateReferencesTo(
        {&secondTarget}, context, combat, false);
    assert(hunter.getCurrentTarget() == nullptr);
}

void verifyMultiEntityCleanup(
    Map& map,
    TextureManager& textures
) {
    CombatManager combat;
    GameContext context;
    context.map = &map;
    context.combatManager = &combat;
    context.deltaTime = 1.f;

    std::vector<std::unique_ptr<Monster>> monsters;
    monsters.push_back(std::make_unique<Monster>(330.f, 330.f));
    monsters.push_back(std::make_unique<Monster>(330.f, 330.f));
    std::vector<std::unique_ptr<Ally>> allies;
    allies.push_back(std::make_unique<Ally>(
        360.f, 330.f, textures, "Damian"));
    allies.push_back(std::make_unique<Ally>(
        390.f, 330.f, textures, "Junior"));

    for (const auto& monster : monsters) {
        context.allEntity.push_back(monster.get());
        context.enemies.push_back(monster.get());
        context.monsters.push_back(monster.get());
    }
    for (const auto& ally : allies) {
        context.allEntity.push_back(ally.get());
        context.players.push_back(ally.get());
        context.allies.push_back(ally.get());
    }

    // Two projectiles kill two overlapping Monsters in one processing pass;
    // projectile erasure must not invalidate either target iteration.
    context.projectiles.push_back(std::make_unique<Arrow>(
        sf::Vector2f{330.f, 330.f},
        sf::Vector2f{},
        0.f,
        1000.f,
        Team::Player
    ));
    context.projectiles.push_back(std::make_unique<Arrow>(
        sf::Vector2f{330.f, 330.f},
        sf::Vector2f{},
        0.f,
        1000.f,
        Team::Player
    ));
    combat.processProjectiles(context, context.allEntity);
    assert(monsters[0]->isDead());
    assert(monsters[1]->isDead());
    assert(context.projectiles.empty());

    // Allies die in the same frame as the Monsters.
    for (const auto& ally : allies) {
        ally->takeDamage(ally->getMaxHealth());
    }

    std::vector<Entity*> dead;
    for (Entity* entity : context.allEntity) {
        if (entity->isDead()) dead.push_back(entity);
    }
    EntityLifecycle::invalidateReferencesTo(
        dead, context, combat, false);
    assert(context.players.empty());
    assert(context.enemies.empty());
    assert(context.monsters.empty());
    assert(context.allies.empty());

    for (const auto& monster : monsters) {
        monster->update(context);
        assert(monster->isReadyToBeDelete());
    }
    for (const auto& ally : allies) {
        ally->update(context);
        assert(ally->isReadyToBeDelete());
    }

    EntityLifecycle::invalidateReferencesTo(
        dead, context, combat, true);
    monsters.erase(
        std::remove_if(
            monsters.begin(), monsters.end(),
            [](const std::unique_ptr<Monster>& monster) {
                return monster->isReadyToBeDelete();
            }),
        monsters.end());
    allies.erase(
        std::remove_if(
            allies.begin(), allies.end(),
            [](const std::unique_ptr<Ally>& ally) {
                return ally->isReadyToBeDelete();
            }),
        allies.end());

    assert(monsters.empty());
    assert(allies.empty());
    assert(context.allEntity.empty());
    assert(combat.getTrackedHitCount() == 0);
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
    assert(!map.collidesWithSolid(first->getCollisionBox()));
    monsters.push_back(std::move(first));

    for (int index = 1; index < expected.count; ++index) {
        assert(!waves.update(0.44f, map, false));
        auto next = waves.update(0.02f, map, false);
        assert(next);
        assert(getEnemyType(*next) == expected.enemyType);
        assert(!map.collidesWithSolid(next->getCollisionBox()));
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
    verifyMapCollisionRules(map);
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

    verifyMovementCollision(map, textures);
    verifySeparation(map, textures);
    verifyTargetAndCacheInvalidation(map, textures);
    verifyMultiEntityCleanup(map, textures);

    const auto correctedAllyPosition = map.findNearestValidPosition(
        map.gridToWorld({4, 3}), {20.f, 20.f});
    assert(correctedAllyPosition);
    assert(!map.collidesWithSolid({
        *correctedAllyPosition - sf::Vector2f(20.f, 20.f),
        {40.f, 40.f}
    }));

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
