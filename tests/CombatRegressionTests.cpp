#include "Ally.h"
#include "BalanceConfig.h"
#include "Boss.h"
#include "Bow.h"
#include "CombatManager.h"
#include "EnemyConfig.h"
#include "EntityLifecycle.h"
#include "GameContext.h"
#include "Map.h"
#include "Monster.h"
#include "Player.h"
#include "Sword.h"
#include "TextureManager.h"
#include "UpgradeManager.h"

#include <array>
#include <cassert>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

namespace {
constexpr float EPSILON = 0.0001f;

void assertNear(float actual, float expected) {
    assert(std::abs(actual - expected) <= EPSILON);
}

void loadTextures(TextureManager& textures) {
    assert(textures.loadTexture("PlayerMage", "assets/images/PlayerMage.png"));
    for (const char* name : {"Damian", "Evangeline", "Junior", "Lucas"}) {
        assert(textures.loadTexture(
            name, std::string("assets/images/") + name + ".png"));
    }
}

struct TestWorld {
    Map map{15, 15};
    CombatManager combat;
    GameContext context;

    TestWorld() {
        context.map = &map;
        context.combatManager = &combat;
        context.deltaTime = 0.01f;
    }
};

void configurePlayerBow(Player& player) {
    player.setPosition({300.f, 300.f});
    player.setAttackPower(BalanceConfig::PLAYER_DAMAGE);
    player.setAttackCooldown(0.55f);
    player.setCurrentWeapon(std::make_unique<Bow>());
}

void fireBow(Entity& shooter, TestWorld& world) {
    shooter.setAttackDirection({1.f, 0.f});
    shooter.setIsAttacking(true);
    shooter.getCurrentWeapon()->triggerAction(
        &shooter, world.context, world.combat);
}

std::vector<std::unique_ptr<Ally>> makeAllies(TextureManager& textures) {
    std::vector<std::unique_ptr<Ally>> allies;
    const std::array<AllyType, 4> types{{
        AllyType::Damian, AllyType::Evangeline,
        AllyType::Junior, AllyType::Lucas
    }};
    for (std::size_t i = 0; i < types.size(); ++i) {
        allies.push_back(createAlly(
            types[i], {400.f + static_cast<float>(i) * 40.f, 300.f}, textures));
    }
    return allies;
}

void testR1PlayerBowBaseline(TextureManager& textures) {
    TestWorld world;
    Player player(textures);
    configurePlayerBow(player);
    Monster target(340.f, 300.f, 500.f, 500.f);
    world.context.allEntity = {&player, &target};

    fireBow(player, world);
    assert(world.context.projectiles.size() == 1);
    assertNear(world.context.projectiles[0]->getDamage(), 18.f);
    assert(world.context.projectiles[0]->getShooterTeam() == Team::Player);

    const float before = target.getHealth();
    world.context.deltaTime = 0.03f;
    world.combat.processProjectiles(world.context, world.context.allEntity);
    assertNear(target.getHealth(), before - 18.f);
}

void testR2DamageUpgradeChangesNewShots(TextureManager& textures) {
    TestWorld world;
    Player player(textures);
    configurePlayerBow(player);
    auto allies = makeAllies(textures);
    UpgradeManager upgrades;

    assert(upgrades.purchase(UpgradeType::Damage, player, allies));
    assertNear(player.getAttackPower(), 22.f);
    fireBow(player, world);
    assertNear(world.context.projectiles.back()->getDamage(), 22.f);
}

void testR3ProjectileSnapshotsDamage(TextureManager& textures) {
    TestWorld world;
    Player player(textures);
    configurePlayerBow(player);
    auto allies = makeAllies(textures);
    UpgradeManager upgrades;

    fireBow(player, world);
    player.setIsAttacking(false);
    assert(upgrades.purchase(UpgradeType::Damage, player, allies));
    fireBow(player, world);

    assert(world.context.projectiles.size() == 2);
    assertNear(world.context.projectiles[0]->getDamage(), 18.f);
    assertNear(world.context.projectiles[1]->getDamage(), 22.f);
}

void testR4UndoRestoresDamage(TextureManager& textures) {
    TestWorld world;
    Player player(textures);
    configurePlayerBow(player);
    auto allies = makeAllies(textures);
    std::array<float, 4> base{};
    for (std::size_t i = 0; i < allies.size(); ++i) {
        base[i] = allies[i]->getAttackPower();
    }
    UpgradeManager upgrades;

    assert(upgrades.purchase(UpgradeType::Damage, player, allies));
    assert(upgrades.undoLastPurchase(player, allies));
    assert(!upgrades.undoLastPurchase(player, allies));
    assertNear(player.getAttackPower(), 18.f);
    for (std::size_t i = 0; i < allies.size(); ++i) {
        assertNear(allies[i]->getAttackPower(), base[i]);
    }

    fireBow(player, world);
    assertNear(world.context.projectiles.back()->getDamage(), 18.f);
}

void testR5UpgradePolicyAndConfigIsolation(TextureManager& textures) {
    Player player(textures);
    configurePlayerBow(player);
    auto allies = makeAllies(textures);
    std::array<float, 4> baseDamage{};
    std::array<float, 4> baseHealth{};
    std::array<float, 4> baseCurrentHealth{};
    for (std::size_t i = 0; i < allies.size(); ++i) {
        baseDamage[i] = allies[i]->getAttackPower();
        baseHealth[i] = allies[i]->getMaxHealth();
        baseCurrentHealth[i] = allies[i]->getHealth();
    }
    UpgradeManager upgrades;

    assert(upgrades.purchase(UpgradeType::Damage, player, allies));
    for (std::size_t i = 0; i < allies.size(); ++i) {
        assertNear(allies[i]->getAttackPower(), baseDamage[i] + 3.f);
    }
    auto newLucas = createAlly(AllyType::Lucas, {600.f, 300.f}, textures);
    assertNear(newLucas->getAttackPower(), 38.f);
    assertNear(newLucas->getMaxHealth(), 360.f);
    assert(upgrades.undoLastPurchase(player, allies));
    for (std::size_t i = 0; i < allies.size(); ++i) {
        assertNear(allies[i]->getAttackPower(), baseDamage[i]);
    }

    assert(upgrades.purchase(UpgradeType::Vitality, player, allies));
    for (std::size_t i = 0; i < allies.size(); ++i) {
        assertNear(allies[i]->getMaxHealth(), baseHealth[i] + 15.f);
        assertNear(allies[i]->getHealth(), baseCurrentHealth[i] + 15.f);
    }
    assert(upgrades.undoLastPurchase(player, allies));
    for (std::size_t i = 0; i < allies.size(); ++i) {
        assertNear(allies[i]->getMaxHealth(), baseHealth[i]);
        assertNear(allies[i]->getHealth(), baseCurrentHealth[i]);
    }

    assertNear(getAllyStats(AllyType::Damian).maxHealth, 450.f);
    assert(getAllyStats(AllyType::Damian).attackDamage == 28);
    assertNear(getAllyStats(AllyType::Lucas).maxHealth, 360.f);
    assert(getAllyStats(AllyType::Lucas).attackDamage == 38);
}

void testR6MonsterBaseline(TextureManager& textures) {
    TestWorld world;
    Ally target(340.f, 300.f, textures, AllyType::Damian);
    Monster monster(300.f, 300.f, 500.f, 500.f, 50.f, 1.f, 0.f, 13.f);
    monster.setCurrentWeapon(std::make_unique<Sword>(50.f));
    world.context.players = {&target};
    world.context.enemies = {&monster};

    assertNear(monster.getAttackPower(), 13.f);
    assertNear(monster.getAttackCooldown(), 1.f);
    assertNear(monster.getCurrentWeapon()->getAttackRange(), 50.f);
    const float before = target.getHealth();
    monster.update(world.context);
    assertNear(target.getHealth(), before - 13.f);

    world.context.deltaTime = 0.1f;
    for (int i = 0; i < 3; ++i) monster.update(world.context);
    const float afterFirst = target.getHealth();
    assertNear(afterFirst, before - 13.f);
    world.context.deltaTime = 0.99f;
    monster.update(world.context);
    assertNear(target.getHealth(), afterFirst);
    world.context.deltaTime = 0.02f;
    monster.update(world.context);
    assertNear(target.getHealth(), afterFirst - 13.f);
}

void testR7BossBaselineAndEnrage(TextureManager& textures) {
    TestWorld world;
    Ally target(350.f, 300.f, textures, AllyType::Damian);
    Boss boss(300.f, 300.f, 4);
    boss.setCurrentWeapon(std::make_unique<Sword>(62.f));
    world.context.players = {&target};
    world.context.enemies = {&boss};

    assertNear(boss.getMaxHealth(), 1020.f);
    assertNear(boss.getAttackPower(), 28.f);
    assertNear(boss.getAttackCooldown(), 0.8f);
    assertNear(boss.getCurrentWeapon()->getAttackRange(), 62.f);
    const float before = target.getHealth();
    boss.update(world.context);
    assertNear(target.getHealth(), before - 28.f);

    world.context.deltaTime = 0.1f;
    for (int i = 0; i < 3; ++i) boss.update(world.context);
    const float afterFirst = target.getHealth();
    assertNear(afterFirst, before - 28.f);
    world.context.deltaTime = 0.79f;
    boss.update(world.context);
    assertNear(target.getHealth(), afterFirst);
    world.context.deltaTime = 0.02f;
    boss.update(world.context);
    assertNear(target.getHealth(), afterFirst - 28.f);

    boss.takeDamage(boss.getMaxHealth() * 0.5f);
    boss.update(world.context);
    assert(boss.getPhase() == Boss::Phase::Phase2);
    assertNear(boss.getAttackPower(), 28.f);
    assertNear(boss.getAttackCooldown(), 0.8f);
    assertNear(boss.getCurrentWeapon()->getAttackRange(), 62.f);

    target.takeDamage(target.getMaxHealth());
    boss.update(world.context);
    assert(boss.getCurrentTarget() == nullptr);
}

void testR8FriendlyFire(TextureManager& textures) {
    TestWorld world;
    Player player(textures);
    configurePlayerBow(player);
    Ally ally(315.f, 300.f, textures, AllyType::Lucas);
    Monster monster(340.f, 300.f, 500.f, 500.f);
    world.context.allEntity = {&player, &ally, &monster};
    const float allyHealth = ally.getHealth();
    const float monsterHealth = monster.getHealth();

    fireBow(player, world);
    world.context.deltaTime = 0.03f;
    world.combat.processProjectiles(world.context, world.context.allEntity);
    assertNear(ally.getHealth(), allyHealth);
    assertNear(monster.getHealth(), monsterHealth - 18.f);

    TestWorld allyWorld;
    Player friendlyPlayer(textures);
    friendlyPlayer.setPosition({315.f, 300.f});
    Ally archer(300.f, 300.f, textures, AllyType::Damian);
    Ally friendlyAlly(320.f, 300.f, textures, AllyType::Lucas);
    Monster enemy(340.f, 300.f, 500.f, 500.f);
    allyWorld.context.allEntity = {
        &archer, &friendlyPlayer, &friendlyAlly, &enemy};
    const float playerHealth = friendlyPlayer.getHealth();
    const float friendlyAllyHealth = friendlyAlly.getHealth();
    fireBow(archer, allyWorld);
    allyWorld.context.deltaTime = 0.03f;
    allyWorld.combat.processProjectiles(
        allyWorld.context, allyWorld.context.allEntity);
    assertNear(friendlyPlayer.getHealth(), playerHealth);
    assertNear(friendlyAlly.getHealth(), friendlyAllyHealth);
    assertNear(enemy.getHealth(), 500.f - 28.f);
}

void testR9ProjectileOutlivesShooter(TextureManager& textures) {
    TestWorld world;
    auto shooter = std::make_unique<Player>(textures);
    configurePlayerBow(*shooter);
    Monster target(340.f, 300.f, 500.f, 500.f);
    fireBow(*shooter, world);
    assertNear(world.context.projectiles[0]->getDamage(), 18.f);
    assert(world.context.projectiles[0]->getShooterTeam() == Team::Player);
    shooter.reset();

    world.context.allEntity = {&target};
    world.context.deltaTime = 0.03f;
    world.combat.processProjectiles(world.context, world.context.allEntity);
    assertNear(target.getHealth(), 482.f);
}

void testR10ProjectileOutlivesTarget(TextureManager& textures) {
    TestWorld world;
    Player shooter(textures);
    configurePlayerBow(shooter);
    auto oldTarget = std::make_unique<Monster>(500.f, 300.f);
    world.context.allEntity = {&shooter, oldTarget.get()};
    world.context.enemies = {oldTarget.get()};
    world.context.monsters = {oldTarget.get()};
    fireBow(shooter, world);

    oldTarget->takeDamage(oldTarget->getMaxHealth());
    EntityLifecycle::invalidateReferencesTo(
        {oldTarget.get()}, world.context, world.combat, true);
    oldTarget.reset();
    Monster replacement(340.f, 300.f, 500.f, 500.f);
    world.context.allEntity = {&shooter, &replacement};
    world.context.deltaTime = 0.03f;
    world.combat.processProjectiles(world.context, world.context.allEntity);
    assertNear(replacement.getHealth(), 482.f);
}

void testR11DeathCancelsCombat(TextureManager& textures) {
    TestWorld world;
    Player player(textures);
    configurePlayerBow(player);
    player.setIsAttacking(true);
    player.takeDamage(player.getMaxHealth());
    assert(!player.getIsAttacking());
    assert(player.getIsDying());
    assert(!player.getCurrentWeapon()->getHasAttacked());
    player.getCurrentWeapon()->triggerAction(&player, world.context, world.combat);
    assert(world.context.projectiles.empty());

    Ally dyingAlly(300.f, 350.f, textures, AllyType::Damian);
    Monster allyTarget(340.f, 350.f, 500.f, 500.f);
    world.context.enemies = {&allyTarget};
    dyingAlly.updateTarget(world.context);
    assert(dyingAlly.getTarget() == &allyTarget);
    dyingAlly.setIsAttacking(true);
    dyingAlly.takeDamage(dyingAlly.getMaxHealth());
    assert(!dyingAlly.getIsAttacking());
    assert(!dyingAlly.getCurrentWeapon()->getHasAttacked());
    const float allyTargetHealth = allyTarget.getHealth();
    dyingAlly.update(world.context);
    assert(dyingAlly.getTarget() == nullptr);
    assertNear(allyTarget.getHealth(), allyTargetHealth);
    assert(dyingAlly.getIsDying());

    Ally ally(340.f, 300.f, textures, AllyType::Damian);
    Monster enemy(300.f, 300.f, 500.f, 500.f, 50.f, 1.f, 0.f, 13.f);
    enemy.setCurrentWeapon(std::make_unique<Sword>(50.f));
    world.context.players = {&ally};
    world.context.enemies = {&enemy};
    enemy.updateTarget(world.context.players);
    assert(enemy.getCurrentTarget() == &ally);
    enemy.setIsAttacking(true);
    enemy.takeDamage(enemy.getMaxHealth());
    assert(!enemy.getIsAttacking());
    assert(!enemy.getCurrentWeapon()->getHasAttacked());
    enemy.update(world.context);
    assert(enemy.getCurrentTarget() == nullptr);
    assert(enemy.getIsDying());
    assert(!enemy.isReadyToBeDelete());
}

void testR12CooldownOncePerCycle(TextureManager& textures) {
    TestWorld world;
    Player player(textures);
    configurePlayerBow(player);
    fireBow(player, world);
    world.context.deltaTime = 0.1f;
    for (int i = 0; i < 10; ++i) player.update(world.context);
    assert(world.context.projectiles.size() == 1);
    assert(!player.canAttack());
    world.context.deltaTime = 0.54f;
    player.update(world.context);
    assert(!player.canAttack());
    world.context.deltaTime = 0.02f;
    player.update(world.context);
    assert(player.canAttack());

    Ally ally(300.f, 300.f, textures, AllyType::Lucas);
    Monster target(340.f, 300.f, 1000.f, 1000.f);
    world.context.players = {&ally};
    world.context.enemies = {&target};
    const float before = target.getHealth();
    world.context.deltaTime = 0.01f;
    ally.update(world.context);
    assertNear(target.getHealth(), before - 38.f);
    world.context.deltaTime = 0.1f;
    for (int i = 0; i < 10; ++i) ally.update(world.context);
    const float afterFirst = target.getHealth();
    world.context.deltaTime = ally.getAttackCooldown() - 0.01f;
    ally.update(world.context);
    assertNear(target.getHealth(), afterFirst);
    world.context.deltaTime = 0.02f;
    ally.update(world.context);
    assertNear(target.getHealth(), afterFirst - 38.f);

    Monster attacker(300.f, 300.f, 500.f, 500.f, 50.f, 0.7f, 0.f, 9.f);
    attacker.setCurrentWeapon(std::make_unique<Sword>(50.f));
    Ally victim(340.f, 300.f, textures, AllyType::Damian);
    world.context.players = {&victim};
    world.context.enemies = {&attacker};
    const float victimBefore = victim.getHealth();
    world.context.deltaTime = 0.01f;
    attacker.update(world.context);
    assertNear(victim.getHealth(), victimBefore - 9.f);
    world.context.deltaTime = 0.1f;
    for (int i = 0; i < 3; ++i) attacker.update(world.context);
    const float victimAfterFirst = victim.getHealth();
    world.context.deltaTime = 0.69f;
    attacker.update(world.context);
    assertNear(victim.getHealth(), victimAfterFirst);
    world.context.deltaTime = 0.02f;
    attacker.update(world.context);
    assertNear(victim.getHealth(), victimAfterFirst - 9.f);
}
}

int main() {
    TextureManager textures;
    loadTextures(textures);
    testR1PlayerBowBaseline(textures);
    testR2DamageUpgradeChangesNewShots(textures);
    testR3ProjectileSnapshotsDamage(textures);
    testR4UndoRestoresDamage(textures);
    testR5UpgradePolicyAndConfigIsolation(textures);
    testR6MonsterBaseline(textures);
    testR7BossBaselineAndEnrage(textures);
    testR8FriendlyFire(textures);
    testR9ProjectileOutlivesShooter(textures);
    testR10ProjectileOutlivesTarget(textures);
    testR11DeathCancelsCombat(textures);
    testR12CooldownOncePerCycle(textures);
    return 0;
}
