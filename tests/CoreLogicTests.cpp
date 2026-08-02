#include "Ally.h"
#include "Arrow.h"
#include "BalanceConfig.h"
#include "Bow.h"
#include "Boss.h"
#include "CombatManager.h"
#include "Constants.h"
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
#include <random>
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

void verifyMapCollisionRules(Map& map);

void verifyMapVisualLayer(TextureManager& textures) {
    using namespace GameConfig;
    static_assert(TILE_SIZE == 48.f);

    assert(mapTextureKeys(TILE_GRASS)[0] == "MapGrass01");
    assert(mapTextureKeys(TILE_GRASS)[1] == "MapGrass02");
    assert(mapTextureKeys(TILE_DEPLOY) == mapTextureKeys(TILE_GRASS));
    assert(mapTextureKeys(TILE_PATH)[0] == "MapPath01");
    assert(mapTextureKeys(TILE_SPAWN) == mapTextureKeys(TILE_PATH));
    assert(mapTextureKeys(TILE_WALL)[1] == "MapWall02");

    for (const TileType type : {TILE_GRASS, TILE_PATH, TILE_WALL}) {
        for (int row = 0; row < 12; ++row) {
            for (int column = 0; column < 12; ++column) {
                const std::size_t first =
                    selectTileVariant(row, column, type, 2);
                const std::size_t second =
                    selectTileVariant(row, column, type, 2);
                assert(first == second);
                assert(first < 2);
            }
        }
    }

    std::mt19937 observedRng(0x51E6Eu);
    std::mt19937 controlRng(0x51E6Eu);
    (void)selectTileVariant(9, 4, TILE_PATH, 2);
    assert(observedRng() == controlRng());

    Map first(15, 15);
    Map second(15, 15);
    std::vector<TileType> snapshot;
    for (int row = 0; row < first.getHeight(); ++row) {
        for (int column = 0; column < first.getWidth(); ++column) {
            const TileType tile = first.getTileType(column, row);
            snapshot.push_back(tile);
            assert(tile == second.getTileType(column, row));
            assert(first.isWalkable(column, row) ==
                   second.isWalkable(column, row));
        }
    }

    first.setTextureManager(textures);
    for (int row = 0; row < first.getHeight(); ++row) {
        for (int column = 0; column < first.getWidth(); ++column) {
            const std::size_t index = static_cast<std::size_t>(
                row * first.getWidth() + column);
            assert(first.getTileType(column, row) == snapshot[index]);
        }
    }

    for (const TileType type : {TILE_GRASS, TILE_PATH, TILE_WALL}) {
        for (const std::string_view key : mapTextureKeys(type)) {
            const sf::Texture* texture =
                textures.findTexture(std::string(key));
            assert(texture != nullptr);
            assert(texture->getSize() ==
                   sf::Vector2u(static_cast<unsigned>(TILE_SIZE),
                                static_cast<unsigned>(TILE_SIZE)));
        }
    }

    verifyMapCollisionRules(first);
    for (const sf::Vector2i spawn : first.getEnemySpawnCells()) {
        assert(!first.findPathBFS(
            spawn, {first.getWidth() / 2, first.getHeight() / 2}).empty());
    }
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
    // C4: a loaded/spawned actor may begin a fraction inside a wall.
    // Collision resolution must depenetrate it even without input.
    constexpr sf::Vector2f playerHalfExtents{18.f, 18.f};
    const sf::Vector2f overlappingWall{177.f, 160.f};
    assert(map.collidesWithSolid({
        overlappingWall - playerHalfExtents,
        playerHalfExtents * 2.f
    }));
    const sf::Vector2f recovered = map.resolveMovement(
        overlappingWall, playerHalfExtents, {});
    assertFinite(recovered);
    assert(!map.collidesWithSolid({
        recovered - playerHalfExtents,
        playerHalfExtents * 2.f
    }));

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

    // C2: collide with the outside-wall corner. Both axes stay finite and
    // solid, while the earlier diagonal case proves sliding on a free axis.
    player.setPosition({50.f, 50.f});
    player.moveWithCollision({-300.f, -250.f}, map);
    assertFinite(player.getPosition());
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
        variant->setCurrentWeapon(std::make_unique<Sword>(50.f));
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

void verifyCrowdSeparation(
    TextureManager& textures
) {
    Map spaciousMap(60, 20);
    constexpr std::size_t crowdSize = 20;
    std::vector<std::unique_ptr<Monster>> firstRun;
    std::vector<std::unique_ptr<Monster>> secondRun;
    std::vector<Monster*> firstPointers;
    std::vector<Monster*> secondPointers;
    for (std::size_t index = 0; index < crowdSize; ++index) {
        const float x = 600.f + static_cast<float>(index) * 20.f;
        firstRun.push_back(std::make_unique<Monster>(x, 240.f));
        secondRun.push_back(std::make_unique<Monster>(x, 240.f));
        firstPointers.push_back(firstRun.back().get());
        secondPointers.push_back(secondRun.back().get());
    }

    std::vector<Ally*> noAllies;
    for (int pass = 0; pass < 12; ++pass) {
        std::vector<sf::Vector2f> before;
        for (Monster* monster : firstPointers) {
            before.push_back(monster->getPosition());
        }
        EntityCollision::separateLivingEntities(
            spaciousMap, firstPointers, noAllies);
        EntityCollision::separateLivingEntities(
            spaciousMap, secondPointers, noAllies);

        for (std::size_t index = 0; index < crowdSize; ++index) {
            const sf::Vector2f position =
                firstPointers[index]->getPosition();
            assertFinite(position);
            assert(!spaciousMap.collidesWithSolid(
                firstPointers[index]->getCollisionBox()));
            const sf::Vector2f movement = position - before[index];
            assert(std::abs(movement.x) <= 96.f);
            assert(std::abs(movement.y) <= 96.f);
            assert(position == secondPointers[index]->getPosition());
        }
    }

    // O3/C5: crowd a real 32 px gate. Wall safety has priority over forcing
    // every overlap to zero when the doorway has insufficient space.
    Map doorwayMap(15, 15);
    std::vector<std::unique_ptr<Monster>> doorwayOwners;
    std::vector<Monster*> doorwayMonsters;
    for (int index = 0; index < 12; ++index) {
        doorwayOwners.push_back(
            std::make_unique<Monster>(256.f, 216.f));
        doorwayMonsters.push_back(doorwayOwners.back().get());
    }
    for (int pass = 0; pass < 20; ++pass) {
        std::vector<sf::Vector2f> before;
        for (Monster* monster : doorwayMonsters) {
            before.push_back(monster->getPosition());
        }
        EntityCollision::separateLivingEntities(
            doorwayMap, doorwayMonsters, noAllies);
        for (std::size_t index = 0;
             index < doorwayMonsters.size();
             ++index) {
            Monster* monster = doorwayMonsters[index];
            assertFinite(monster->getPosition());
            assert(!doorwayMap.collidesWithSolid(
                monster->getCollisionBox()));
            const sf::Vector2f movement =
                monster->getPosition() - before[index];
            assert(std::abs(movement.x) <= 96.f);
            assert(std::abs(movement.y) <= 96.f);
        }
    }

    // O4: a corpse waiting for its death timer is ignored by separation.
    const sf::Vector2f corpsePosition{500.f, 400.f};
    Monster corpse(corpsePosition.x, corpsePosition.y);
    Monster living(corpsePosition.x, corpsePosition.y);
    corpse.takeDamage(corpse.getMaxHealth());
    const float deadHealth = corpse.getHealth();
    corpse.takeDamage(1000.f);
    std::vector<Monster*> livingAndDead{&corpse, &living};
    EntityCollision::separateLivingEntities(
        spaciousMap, livingAndDead, noAllies);
    assert(corpse.getPosition() == corpsePosition);
    assert(living.getPosition() == corpsePosition);
    assert(corpse.getHealth() == deadHealth);
    corpse.setIsAttacking(true);
    assert(!corpse.getIsAttacking());

    Ally observer(550.f, 400.f, textures, "Damian");
    GameContext targetContext;
    targetContext.enemies = {&corpse};
    observer.updateTarget(targetContext);
    assert(observer.getTarget() == nullptr);
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
    attacker.setAttackPower(1000.f);
    attacker.setCurrentWeapon(std::make_unique<Sword>(105.f));
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

void verifyTargetPoliciesAndAdjacentDeaths(
    Map& map,
    TextureManager& textures
) {
    CombatManager combat;
    GameContext context;
    context.map = &map;
    context.combatManager = &combat;

    Ally ally(300.f, 300.f, textures, "Damian");
    Monster outsideRange(600.f, 300.f);
    context.enemies = {&outsideRange};
    ally.updateTarget(context);
    assert(ally.getTarget() == nullptr);

    // T4: Monsters retain the nearest living candidate regardless of range.
    Monster hunter(100.f, 100.f);
    Ally farTarget(700.f, 300.f, textures, "Junior");
    hunter.updateTarget({&farTarget});
    assert(hunter.getCurrentTarget() == &farTarget);

    // T3/T6: adjacent dead candidates are invalidated together and cannot be
    // selected again even while they remain in the death-animation snapshot.
    Monster first(330.f, 300.f);
    Monster second(340.f, 300.f);
    Monster survivor(350.f, 300.f);
    context.allEntity = {&ally, &first, &second, &survivor};
    context.players = {&ally};
    context.allies = {&ally};
    context.enemies = {&first, &second, &survivor};
    context.monsters = {&first, &second, &survivor};
    ally.updateTarget(context);
    assert(ally.getTarget() == &first);

    first.takeDamage(first.getMaxHealth());
    second.takeDamage(second.getMaxHealth());
    EntityLifecycle::invalidateReferencesTo(
        {&first, &second}, context, combat, false);
    assert(!containsPointer(context.enemies, &first));
    assert(!containsPointer(context.enemies, &second));
    assert(containsPointer(context.allEntity, &first));
    assert(containsPointer(context.allEntity, &second));
    ally.updateTarget(context);
    assert(ally.getTarget() == &survivor);

    context.enemies = {&first, &second};
    ally.updateTarget(context);
    assert(ally.getTarget() == nullptr);
}

void verifyBulkLifecycleAndProjectile(
    TextureManager& textures
) {
    Map map(60, 20);
    CombatManager combat;
    GameContext context;
    context.map = &map;
    context.combatManager = &combat;
    context.deltaTime = 0.6f;

    constexpr std::size_t entityCount = 25;
    std::vector<std::unique_ptr<Monster>> monsters;
    for (std::size_t index = 0; index < entityCount; ++index) {
        monsters.push_back(std::make_unique<Monster>(
            600.f + static_cast<float>(index) * 36.f,
            300.f
        ));
        context.allEntity.push_back(monsters.back().get());
        context.enemies.push_back(monsters.back().get());
        context.monsters.push_back(monsters.back().get());
    }

    Ally observer(550.f, 300.f, textures, "Lucas");
    context.allEntity.insert(context.allEntity.begin(), &observer);
    context.players = {&observer};
    context.allies = {&observer};
    observer.updateTarget(context);

    std::vector<Entity*> dead;
    std::size_t expectedSurvivors = 0;
    for (std::size_t index = 0; index < monsters.size(); ++index) {
        if (index % 3 == 0) {
            ++expectedSurvivors;
        } else {
            monsters[index]->takeDamage(
                monsters[index]->getMaxHealth());
            dead.push_back(monsters[index].get());
        }
    }
    assert(dead.size() >= 10);

    EntityLifecycle::invalidateReferencesTo(
        dead, context, combat, false);
    EntityLifecycle::invalidateReferencesTo(
        dead, context, combat, false);
    assert(context.enemies.size() == expectedSurvivors);
    assert(context.monsters.size() == expectedSurvivors);

    int firstRewardTotal = 0;
    int repeatedRewardTotal = 0;
    for (const auto& monster : monsters) {
        firstRewardTotal += monster->claimGoldReward();
        repeatedRewardTotal += monster->claimGoldReward();
        monster->update(context);
    }
    assert(firstRewardTotal == static_cast<int>(dead.size()) * 10);
    assert(repeatedRewardTotal == 0);

    // L4: projectiles store position/team data, not a target pointer. One may
    // safely outlive removal of unrelated target objects.
    context.projectiles.push_back(std::make_unique<Arrow>(
        sf::Vector2f{1200.f, 500.f},
        sf::Vector2f{1.f, 0.f},
        50.f,
        10.f,
        Team::Player
    ));

    EntityLifecycle::invalidateReferencesTo(
        dead, context, combat, true);
    monsters.erase(
        std::remove_if(
            monsters.begin(), monsters.end(),
            [](const std::unique_ptr<Monster>& monster) {
                return monster->isReadyToBeDelete();
            }),
        monsters.end());
    assert(monsters.size() == expectedSurvivors);
    assert(context.allEntity.size() == expectedSurvivors + 1);

    combat.processProjectiles(context, context.allEntity);
    assert(context.projectiles.size() == 1);
    assert(context.projectiles.front()->isActive());
}

void verifyDeterministicLifecycleStress(
    TextureManager& textures
) {
    Map map(60, 20);
    CombatManager combat;
    GameContext context;
    context.map = &map;
    context.combatManager = &combat;
    context.deltaTime = 0.6f;
    std::mt19937 random(2508);
    std::uniform_real_distribution<float> xPosition(600.f, 1200.f);
    std::uniform_real_distribution<float> yPosition(100.f, 500.f);

    std::vector<std::unique_ptr<Monster>> monsters;
    for (int index = 0; index < 30; ++index) {
        monsters.push_back(std::make_unique<Monster>(
            xPosition(random), yPosition(random)));
    }
    std::vector<std::unique_ptr<Ally>> allies;
    for (int index = 0; index < 4; ++index) {
        allies.push_back(std::make_unique<Ally>(
            1300.f,
            180.f + static_cast<float>(index) * 70.f,
            textures,
            "Damian"
        ));
    }

    for (int round = 0; round < 6; ++round) {
        context.allEntity.clear();
        context.players.clear();
        context.enemies.clear();
        context.monsters.clear();
        context.allies.clear();
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

        EntityCollision::separateLivingEntities(
            map, context.monsters, context.allies);
        for (Monster* monster : context.monsters) {
            assertFinite(monster->getPosition());
            assert(!map.collidesWithSolid(monster->getCollisionBox()));
            monster->updateTarget(context.players);
            assert(monster->getCurrentTarget());
        }
        for (Ally* ally : context.allies) {
            ally->updateTarget(context);
        }

        std::shuffle(
            context.monsters.begin(), context.monsters.end(), random);
        const std::size_t deathsThisRound =
            std::min<std::size_t>(5, context.monsters.size());
        std::vector<Entity*> dead;
        for (std::size_t index = 0; index < deathsThisRound; ++index) {
            Monster* monster = context.monsters[index];
            monster->takeDamage(monster->getMaxHealth());
            dead.push_back(monster);
        }
        EntityLifecycle::invalidateReferencesTo(
            dead, context, combat, false);
        for (Entity* entity : dead) {
            auto* monster = static_cast<Monster*>(entity);
            assert(monster->claimGoldReward() == 10);
            assert(monster->claimGoldReward() == 0);
            monster->update(context);
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
        assert(monsters.size() ==
               30 - static_cast<std::size_t>((round + 1) * 5));
    }
    assert(monsters.empty());
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
        assert(monster.getAttackPower() == stats.effectiveDamage);
    } else if (monster.isElite()) {
        const EnemyConfig::Stats boss =
            EnemyConfig::bossStats(waveNumber);
        assert(monster.getMaxHealth() == boss.maxHealth * 0.5f);
        assert(monster.getHealth() == monster.getMaxHealth());
        assert(monster.getAttackPower() ==
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
    assert(textures.loadTexture("PlayerMage", "assets/images/PlayerMage.png"));
    verifyMapVisualLayer(textures);

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
    verifyCrowdSeparation(textures);
    verifyTargetAndCacheInvalidation(map, textures);
    verifyTargetPoliciesAndAdjacentDeaths(map, textures);
    verifyMultiEntityCleanup(map, textures);
    verifyBulkLifecycleAndProjectile(textures);
    verifyDeterministicLifecycleStress(textures);

    const auto correctedAllyPosition = map.findNearestValidPosition(
        map.gridToWorld({4, 3}), {20.f, 20.f});
    assert(correctedAllyPosition);
    assert(!map.collidesWithSolid({
        *correctedAllyPosition - sf::Vector2f(20.f, 20.f),
        {40.f, 40.f}
    }));

    Player player(textures);
    player.setAttackPower(
        static_cast<float>(BalanceConfig::PLAYER_DAMAGE));
    player.setCurrentWeapon(std::make_unique<Bow>());
    player.setAttackCooldown(0.55f);
    assert(player.getHealth() == BalanceConfig::PLAYER_MAX_HEALTH);
    assert(player.getMaxHealth() == BalanceConfig::PLAYER_MAX_HEALTH);
    assert(player.getAttackPower() ==
           BalanceConfig::PLAYER_DAMAGE);

    const std::array<AllyType, 4> allyTypes{{
        AllyType::Damian,
        AllyType::Evangeline,
        AllyType::Junior,
        AllyType::Lucas
    }};
    std::vector<std::unique_ptr<Ally>> allies;
    for (std::size_t index = 0; index < allyNames.size(); ++index) {
        auto ally = createAlly(
            allyTypes[index],
            {
                100.f + static_cast<float>(index) * 50.f,
                100.f
            },
            textures
        );
        const AllyStats& stats = getAllyStats(allyTypes[index]);
        assert(ally->getType() == allyTypes[index]);
        assert(ally->getHealth() == stats.maxHealth);
        assert(ally->getMaxHealth() == stats.maxHealth);
        assert(ally->getAttackPower() == stats.attackDamage);
        allies.push_back(std::move(ally));
    }

    std::vector<float> initialAllyDamage;
    for (const auto& ally : allies) {
        initialAllyDamage.push_back(ally->getAttackPower());
    }

    UpgradeManager upgrades;
    const int initialGold = upgrades.getGold();

    assert(upgrades.purchase(UpgradeType::Damage, player, allies));
    assert(player.getAttackPower() ==
           BalanceConfig::PLAYER_DAMAGE + 4);
    for (std::size_t index = 0; index < allies.size(); ++index) {
        assert(allies[index]->getAttackPower() ==
               initialAllyDamage[index] + 3.f);
    }
    assert(upgrades.undoLastPurchase(player, allies));
    assert(player.getAttackPower() ==
           BalanceConfig::PLAYER_DAMAGE);
    for (std::size_t index = 0; index < allies.size(); ++index) {
        assert(allies[index]->getAttackPower() ==
               initialAllyDamage[index]);
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
    restartedPlayer.setAttackPower(
        static_cast<float>(BalanceConfig::PLAYER_DAMAGE));
    restartedPlayer.setCurrentWeapon(std::make_unique<Bow>());
    assert(restartedPlayer.getMaxHealth() ==
           BalanceConfig::PLAYER_MAX_HEALTH);
    assert(restartedPlayer.getAttackPower() ==
           BalanceConfig::PLAYER_DAMAGE);

    Ally restartedAlly(
        100.f, 100.f, textures, AllyType::Damian);
    assert(restartedAlly.getMaxHealth() ==
           getAllyStats(AllyType::Damian).maxHealth);
    assert(restartedAlly.getAttackPower() ==
           getAllyStats(AllyType::Damian).attackDamage);
    return 0;
}
