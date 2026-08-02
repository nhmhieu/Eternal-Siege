#include "Ally.h"
#include "Bow.h"
#include "CombatManager.h"
#include "GameContext.h"
#include "Map.h"
#include "Monster.h"
#include "Player.h"
#include "Sword.h"
#include "TextureManager.h"
#include "UpgradeManager.h"
#include "Wand.h"

#include <array>
#include <cassert>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

namespace {
constexpr float EPSILON = 0.0001f;

void assertNear(float actual, float expected) {
    if (std::abs(actual - expected) > EPSILON) {
        std::cerr << "assertNear actual=" << actual
                  << " expected=" << expected << '\n';
    }
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
        context.allySkillsEnabled = true;
        context.deltaTime = 0.01f;
    }
};

Monster makeMonster(float x, float y, float health = 500.f) {
    return Monster(x, y, health, health, 50.f, 1.f, 0.f, 10.f);
}

void testS1SkillMappingAndBaseline(TextureManager& textures) {
    const std::array<AllyType, 4> types{{
        AllyType::Damian, AllyType::Evangeline,
        AllyType::Junior, AllyType::Lucas
    }};
    const std::array<const char*, 4> names{{
        "Explosive Arrow", "Shield Wall", "Arcane Burst", "Whirlwind"
    }};
    const std::array<float, 4> health{{450.f, 400.f, 324.f, 360.f}};
    const std::array<float, 4> damage{{28.f, 24.f, 17.f, 38.f}};
    const std::array<float, 4> cooldown{{1.75f, 2.f, 1.05f, 2.7f}};
    const std::array<float, 4> ranges{{250.f, 115.f, 220.f, 135.f}};
    const std::array<float, 4> skillCooldown{{9.f, 12.f, 7.f, 9.f}};

    for (std::size_t i = 0; i < types.size(); ++i) {
        Ally ally(300.f, 300.f, textures, types[i]);
        assert(ally.getAllyType() == types[i]);
        assert(std::string(ally.getSkillName()) == names[i]);
        assertNear(ally.getMaxHealth(), health[i]);
        assertNear(ally.getAttackPower(), damage[i]);
        assertNear(ally.getAttackCooldown(), cooldown[i]);
        assertNear(ally.getSkillCooldownDuration(), skillCooldown[i]);
        assertNear(ally.getSkillCooldownRemaining(), 0.f);
        assert(!ally.isSkillActive());

        const WeaponType weapon = getAllyStats(types[i]).weaponType;
        assert((dynamic_cast<Sword*>(ally.getCurrentWeapon()) != nullptr) ==
               (weapon == WeaponType::Sword));
        assert((dynamic_cast<Bow*>(ally.getCurrentWeapon()) != nullptr) ==
               (weapon == WeaponType::Bow));
        assert((dynamic_cast<Wand*>(ally.getCurrentWeapon()) != nullptr) ==
               (weapon == WeaponType::Wand));
        assertNear(ally.getAttackRange(), ranges[i]);
    }
}

void testS2ShieldThreshold(TextureManager& textures) {
    TestWorld world;
    Ally evangeline(300.f, 300.f, textures, AllyType::Evangeline);
    evangeline.takeDamage(346.f);
    assertNear(evangeline.getHealth(), 140.5f);
    evangeline.update(world.context);
    assert(!evangeline.isSkillActive());

    evangeline.takeDamage(1.f);
    assertNear(evangeline.getHealth(), 139.75f);
    evangeline.update(world.context);
    assert(evangeline.isSkillActive());
    assertNear(evangeline.getSkillCooldownRemaining(), 12.f);

    Ally dead(300.f, 300.f, textures, AllyType::Evangeline);
    dead.takeDamage(10000.f);
    dead.update(world.context);
    assert(!dead.isSkillActive());
    assertNear(dead.getSkillCooldownRemaining(), 0.f);
}

void testS3ShieldDamageReduction(TextureManager& textures) {
    TestWorld world;
    Ally evangeline(300.f, 300.f, textures, AllyType::Evangeline);
    evangeline.takeDamage(350.f);
    evangeline.update(world.context);
    assert(evangeline.isSkillActive());

    const float before = evangeline.getHealth();
    evangeline.takeDamage(100.f);
    assertNear(evangeline.getHealth(), before - 41.25f);
    evangeline.update(world.context);
    const float beforeSecond = evangeline.getHealth();
    evangeline.takeDamage(20.f);
    assertNear(evangeline.getHealth(), beforeSecond - 8.25f);

    world.context.deltaTime = 4.f;
    evangeline.update(world.context);
    assert(!evangeline.isSkillActive());
    const float afterShield = evangeline.getHealth();
    evangeline.takeDamage(20.f);
    assertNear(evangeline.getHealth(), afterShield - 15.f);
}

void testS4ShieldDoesNotReplaceWeapon(TextureManager& textures) {
    TestWorld world;
    Ally evangeline(300.f, 300.f, textures, AllyType::Evangeline);
    Monster enemy = makeMonster(340.f, 300.f);
    world.context.enemies = {&enemy};
    evangeline.takeDamage(350.f);
    evangeline.update(world.context);

    assert(evangeline.isSkillActive());
    assert(evangeline.getIsAttacking());
    assert(dynamic_cast<Sword*>(evangeline.getCurrentWeapon()));
    assert(world.context.projectiles.empty());
    assertNear(enemy.getHealth(), 500.f - 24.f);
}

void testS5ArcaneBurstCondition(TextureManager& textures) {
    TestWorld oneWorld;
    Ally oneJunior(300.f, 300.f, textures, AllyType::Junior);
    Monster only = makeMonster(450.f, 300.f);
    oneWorld.context.enemies = {&only};
    oneJunior.update(oneWorld.context);
    assert(!oneJunior.isSkillActive());
    assertNear(only.getHealth(), 500.f);

    TestWorld world;
    Ally junior(300.f, 300.f, textures, AllyType::Junior);
    Monster first = makeMonster(400.f, 300.f);
    Monster second = makeMonster(450.f, 300.f);
    Monster outside = makeMonster(471.f, 300.f);
    world.context.enemies = {&first, &second, &outside};
    junior.update(world.context);

    assert(junior.isSkillActive());
    assertNear(first.getHealth(), 476.f);
    assertNear(second.getHealth(), 476.f);
    assertNear(outside.getHealth(), 500.f);
    assertNear(junior.getSkillCooldownRemaining(), 7.f);
}

void testS6ArcaneBurstSingleHit(TextureManager& textures) {
    TestWorld world;
    Ally junior(300.f, 300.f, textures, AllyType::Junior);
    Monster first = makeMonster(360.f, 300.f);
    Monster second = makeMonster(400.f, 300.f);
    world.context.enemies = {&first, &second};
    junior.update(world.context);
    assertNear(first.getHealth(), 476.f);
    assertNear(second.getHealth(), 476.f);
    assert(world.context.projectiles.empty());

    world.context.deltaTime = 0.05f;
    for (int frame = 0; frame < 4; ++frame) junior.update(world.context);
    assertNear(first.getHealth(), 476.f);
    assertNear(second.getHealth(), 476.f);
    assert(world.context.projectiles.empty());
}

void testS7Whirlwind(TextureManager& textures) {
    TestWorld oneWorld;
    Ally oneLucas(300.f, 300.f, textures, AllyType::Lucas);
    Monster only = makeMonster(400.f, 300.f);
    oneWorld.context.enemies = {&only};
    oneLucas.update(oneWorld.context);
    assert(!oneLucas.isSkillActive());
    assert(oneWorld.context.projectiles.empty());
    assertNear(only.getHealth(), 462.f);

    TestWorld world;
    Ally lucas(300.f, 300.f, textures, AllyType::Lucas);
    Monster first = makeMonster(350.f, 300.f);
    Monster second = makeMonster(400.f, 300.f);
    Monster outside = makeMonster(421.f, 300.f);
    world.context.enemies = {&first, &second, &outside};
    lucas.update(world.context);

    assert(lucas.isSkillActive());
    assertNear(first.getHealth(), 458.f);
    assertNear(second.getHealth(), 458.f);
    assertNear(outside.getHealth(), 500.f);
    assert(world.context.projectiles.empty());
    assertNear(lucas.getSkillCooldownRemaining(), 9.f);
}

void testS8ExplosivePrimaryAndSplash(TextureManager& textures) {
    TestWorld world;
    Ally damian(300.f, 300.f, textures, AllyType::Damian);
    Monster primary = makeMonster(340.f, 300.f);
    Monster splash = makeMonster(390.f, 300.f);
    Monster outside = makeMonster(431.f, 300.f);
    world.context.enemies = {&primary, &splash, &outside};
    world.context.allEntity = {&damian, &primary, &splash, &outside};
    damian.update(world.context);

    assert(world.context.projectiles.size() == 1);
    assertNear(world.context.projectiles[0]->getDamage(), 35.f);
    assertNear(world.context.projectiles[0]->getSplashDamage(), 21.f);
    assertNear(world.context.projectiles[0]->getSplashRadius(), 90.f);
    world.context.deltaTime = 0.03f;
    world.combat.processProjectiles(world.context, world.context.allEntity);

    assertNear(primary.getHealth(), 465.f);
    assertNear(splash.getHealth(), 479.f);
    assertNear(outside.getHealth(), 500.f);
    assert(world.context.projectiles.empty());
}

void testS9ExplosiveLifetime(TextureManager& textures) {
    TestWorld shooterWorld;
    auto damian = std::make_unique<Ally>(
        300.f, 300.f, textures, AllyType::Damian);
    Monster target = makeMonster(340.f, 300.f);
    shooterWorld.context.enemies = {&target};
    damian->update(shooterWorld.context);
    assert(shooterWorld.context.projectiles.size() == 1);
    damian.reset();
    shooterWorld.context.allEntity = {&target};
    shooterWorld.context.deltaTime = 0.03f;
    shooterWorld.combat.processProjectiles(
        shooterWorld.context, shooterWorld.context.allEntity);
    assertNear(target.getHealth(), 465.f);

    TestWorld targetWorld;
    Ally secondDamian(300.f, 300.f, textures, AllyType::Damian);
    auto oldTarget = std::make_unique<Monster>(
        500.f, 300.f, 500.f, 500.f, 50.f, 1.f, 0.f, 10.f);
    targetWorld.context.enemies = {oldTarget.get()};
    secondDamian.update(targetWorld.context);
    assert(targetWorld.context.projectiles.size() == 1);
    oldTarget.reset();
    Monster replacement = makeMonster(340.f, 300.f);
    targetWorld.context.enemies = {&replacement};
    targetWorld.context.allEntity = {&secondDamian, &replacement};
    targetWorld.context.deltaTime = 0.03f;
    targetWorld.combat.processProjectiles(
        targetWorld.context, targetWorld.context.allEntity);
    assertNear(replacement.getHealth(), 465.f);
}

void testS10FriendlyFire(TextureManager& textures) {
    TestWorld areaWorld;
    Ally junior(300.f, 300.f, textures, AllyType::Junior);
    Ally friendly(330.f, 300.f, textures, AllyType::Damian);
    Player player(textures);
    player.setPosition({340.f, 300.f});
    Monster first = makeMonster(360.f, 300.f);
    Monster second = makeMonster(400.f, 300.f);
    areaWorld.context.enemies = {
        &friendly, &player, &first, &second};
    const float friendlyHealth = friendly.getHealth();
    const float playerHealth = player.getHealth();
    junior.update(areaWorld.context);
    assertNear(friendly.getHealth(), friendlyHealth);
    assertNear(player.getHealth(), playerHealth);
    assertNear(first.getHealth(), 476.f);
    assertNear(second.getHealth(), 476.f);

    TestWorld projectileWorld;
    Ally damian(300.f, 300.f, textures, AllyType::Damian);
    Ally projectileFriendly(315.f, 300.f, textures, AllyType::Evangeline);
    Monster enemy = makeMonster(340.f, 300.f);
    projectileWorld.context.enemies = {&enemy};
    projectileWorld.context.allEntity = {
        &damian, &projectileFriendly, &enemy};
    const float projectileFriendlyHealth = projectileFriendly.getHealth();
    damian.update(projectileWorld.context);
    projectileWorld.context.deltaTime = 0.03f;
    projectileWorld.combat.processProjectiles(
        projectileWorld.context, projectileWorld.context.allEntity);
    assertNear(projectileFriendly.getHealth(), projectileFriendlyHealth);
    assertNear(enemy.getHealth(), 465.f);
}

void testS11UpgradeUndoAndSnapshots(TextureManager& textures) {
    TestWorld world;
    Player player(textures);
    std::vector<std::unique_ptr<Ally>> allies;
    allies.push_back(createAlly(AllyType::Damian, {300.f, 300.f}, textures));
    allies.push_back(createAlly(AllyType::Evangeline, {500.f, 300.f}, textures));
    allies.push_back(createAlly(AllyType::Junior, {540.f, 300.f}, textures));
    allies.push_back(createAlly(AllyType::Lucas, {580.f, 300.f}, textures));
    Monster enemy = makeMonster(500.f, 300.f, 1000.f);
    world.context.enemies = {&enemy};
    UpgradeManager upgrades;

    allies[0]->update(world.context);
    assert(world.context.projectiles.size() == 1);
    assertNear(world.context.projectiles[0]->getDamage(), 35.f);
    assertNear(world.context.projectiles[0]->getSplashDamage(), 21.f);

    assert(upgrades.purchase(UpgradeType::Damage, player, allies));
    world.context.deltaTime = 9.f;
    allies[0]->update(world.context);
    assert(world.context.projectiles.size() == 2);
    assertNear(world.context.projectiles[0]->getDamage(), 35.f);
    assertNear(world.context.projectiles[1]->getDamage(), 39.f);
    assertNear(world.context.projectiles[1]->getSplashDamage(), 23.f);

    assert(upgrades.undoLastPurchase(player, allies));
    world.context.deltaTime = 9.f;
    allies[0]->update(world.context);
    assert(world.context.projectiles.size() == 3);
    assertNear(world.context.projectiles[2]->getDamage(), 35.f);
    assertNear(allies[0]->getAttackPower(), 28.f);
    assertNear(allies[1]->getAttackPower(), 24.f);
    assertNear(allies[2]->getAttackPower(), 17.f);
    assertNear(allies[3]->getAttackPower(), 38.f);
}

void testS12DeathSafety(TextureManager& textures) {
    TestWorld world;
    Ally junior(300.f, 300.f, textures, AllyType::Junior);
    Monster first = makeMonster(350.f, 300.f);
    Monster second = makeMonster(400.f, 300.f);
    world.context.enemies = {&first, &second};
    junior.takeDamage(junior.getMaxHealth());
    junior.update(world.context);
    assert(!junior.isSkillActive());
    assert(junior.getTarget() == nullptr);
    assertNear(first.getHealth(), 500.f);
    assertNear(second.getHealth(), 500.f);
    assert(world.context.projectiles.empty());

    Ally evangeline(300.f, 300.f, textures, AllyType::Evangeline);
    evangeline.takeDamage(350.f);
    evangeline.update(world.context);
    assert(evangeline.isSkillActive());
    evangeline.takeDamage(10000.f);
    evangeline.update(world.context);
    assert(evangeline.isDead());
    assert(!evangeline.isSkillActive());
    assert(evangeline.getTarget() == nullptr);
}

void triggerSkill(Ally& ally, TestWorld& world,
                  Monster& first, Monster& second) {
    world.context.enemies = {&first, &second};
    if (ally.getAllyType() == AllyType::Evangeline) {
        ally.takeDamage(ally.getMaxHealth());
    }
    ally.update(world.context);
}

void testS13CooldownAndPause(TextureManager& textures) {
    const std::array<AllyType, 4> types{{
        AllyType::Damian, AllyType::Evangeline,
        AllyType::Junior, AllyType::Lucas
    }};
    const std::array<float, 4> cooldowns{{9.f, 12.f, 7.f, 9.f}};

    for (std::size_t i = 0; i < types.size(); ++i) {
        TestWorld world;
        Ally ally(300.f, 300.f, textures, types[i]);
        Monster first = makeMonster(350.f, 300.f, 1000.f);
        Monster second = makeMonster(400.f, 300.f, 1000.f);
        triggerSkill(ally, world, first, second);
        assertNear(ally.getSkillCooldownRemaining(), cooldowns[i]);

        world.context.deltaTime = 1.f;
        ally.update(world.context);
        assertNear(ally.getSkillCooldownRemaining(), cooldowns[i] - 1.f);

        const float pausedValue = ally.getSkillCooldownRemaining();
        assertNear(ally.getSkillCooldownRemaining(), pausedValue);

        world.context.allySkillsEnabled = false;
        world.context.deltaTime = pausedValue - 0.01f;
        ally.update(world.context);
        assert(ally.getSkillCooldownRemaining() > 0.f);
        world.context.deltaTime = 0.02f;
        ally.update(world.context);
        assertNear(ally.getSkillCooldownRemaining(), 0.f);
    }
}

void testS14LegacyContextKeepsCombatIsolation(TextureManager& textures) {
    TestWorld world;
    world.context.allySkillsEnabled = false;
    Ally damian(300.f, 300.f, textures, AllyType::Damian);
    Monster enemy = makeMonster(340.f, 300.f, 1000.f);
    world.context.enemies = {&enemy};
    world.context.allEntity = {&damian, &enemy};
    damian.update(world.context);
    assert(world.context.projectiles.size() == 1);
    world.context.deltaTime = 0.03f;
    world.combat.processProjectiles(world.context, world.context.allEntity);
    assertNear(enemy.getHealth(), 972.f);
    assert(world.context.projectiles.empty());
    assertNear(damian.getSkillCooldownRemaining(), 0.f);
}
}

int main() {
    TextureManager textures;
    loadTextures(textures);
    testS1SkillMappingAndBaseline(textures);
    testS2ShieldThreshold(textures);
    testS3ShieldDamageReduction(textures);
    testS4ShieldDoesNotReplaceWeapon(textures);
    testS5ArcaneBurstCondition(textures);
    testS6ArcaneBurstSingleHit(textures);
    testS7Whirlwind(textures);
    testS8ExplosivePrimaryAndSplash(textures);
    testS9ExplosiveLifetime(textures);
    testS10FriendlyFire(textures);
    testS11UpgradeUndoAndSnapshots(textures);
    testS12DeathSafety(textures);
    testS13CooldownAndPause(textures);
    testS14LegacyContextKeepsCombatIsolation(textures);
    return 0;
}
