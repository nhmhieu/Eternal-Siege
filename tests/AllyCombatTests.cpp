#include "Ally.h"
#include "Bow.h"
#include "CombatManager.h"
#include "EntityLifecycle.h"
#include "GameContext.h"
#include "Map.h"
#include "MagicBolt.h"
#include "Monster.h"
#include "Player.h"
#include "Sword.h"
#include "TextureManager.h"
#include "Wand.h"

#include <array>
#include <cassert>
#include <cmath>
#include <memory>
#include <string>

namespace {
constexpr float EPSILON = 0.0001f;

void assertNear(float actual, float expected) {
    assert(std::abs(actual - expected) <= EPSILON);
}

void loadTextures(TextureManager& textures) {
    assert(textures.loadTexture("PlayerMage", "assets/images/PlayerMage.png"));
    for (const char* name : {
             "Damian", "Evangeline", "Junior", "Lucas"}) {
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

void testA1SupportIsIndependent(TextureManager& textures) {
    TestWorld world;
    Ally protectedAlly(
        300.f, 300.f, textures, AllyType::Damian);
    Ally supportingAlly(
        340.f, 300.f, textures, AllyType::Junior);
    Monster monster(400.f, 300.f, 500.f, 500.f);

    monster.updateTarget({&protectedAlly});
    assert(monster.getCurrentTarget() == &protectedAlly);
    world.context.allEntity = {
        &protectedAlly, &supportingAlly, &monster};
    world.context.players = {&protectedAlly, &supportingAlly};
    world.context.allies = {&protectedAlly, &supportingAlly};
    world.context.enemies = {&monster};
    world.context.monsters = {&monster};

    const float healthBefore = monster.getHealth();
    supportingAlly.update(world.context);
    assert(supportingAlly.getTarget() == &monster);
    assert(monster.getCurrentTarget() == &protectedAlly);
    assert(world.context.projectiles.size() == 1);
    assert(dynamic_cast<MagicBolt*>(world.context.projectiles.front().get()));
    world.context.deltaTime = 0.05f;
    for (int frame = 0; frame < 4 && !world.context.projectiles.empty(); ++frame) {
        world.combat.processProjectiles(world.context, world.context.allEntity);
    }
    assertNear(
        monster.getHealth(),
        healthBefore - supportingAlly.getAttackPower());
}

void testA2OutsideWeaponRange(TextureManager& textures) {
    TestWorld world;
    Ally archer(
        300.f, 300.f, textures, AllyType::Evangeline);
    Monster monster(551.f, 300.f);
    world.context.allEntity = {&archer, &monster};
    world.context.players = {&archer};
    world.context.allies = {&archer};
    world.context.enemies = {&monster};
    world.context.monsters = {&monster};

    const float healthBefore = monster.getHealth();
    archer.update(world.context);
    assert(archer.getTarget() == nullptr);
    assert(!archer.getIsAttacking());
    assert(world.context.projectiles.empty());
    assertNear(monster.getHealth(), healthBefore);
}

void testA3DeadTargetRetargets(TextureManager& textures) {
    TestWorld world;
    Ally archer(
        300.f, 300.f, textures, AllyType::Evangeline);
    Monster first(350.f, 300.f);
    Monster second(360.f, 300.f);
    world.context.allEntity = {&archer, &first, &second};
    world.context.players = {&archer};
    world.context.allies = {&archer};
    world.context.enemies = {&first};
    world.context.monsters = {&first};

    archer.update(world.context);
    assert(archer.getTarget() == &first);
    assert(world.context.projectiles.empty());

    first.takeDamage(first.getMaxHealth());
    EntityLifecycle::invalidateReferencesTo(
        {&first}, world.context, world.combat, false);
    assert(archer.getTarget() == nullptr);
    assert(world.context.enemies.empty());
    world.context.enemies.push_back(&second);
    world.context.monsters.push_back(&second);

    archer.update(world.context);
    assert(archer.getTarget() == &second);
    assert(world.context.projectiles.empty());

    EntityLifecycle::invalidateReferencesTo(
        {&first}, world.context, world.combat, true);
    assert(archer.getTarget() == &second);
}

void testA4DeadAllyCannotFight(TextureManager& textures) {
    TestWorld world;
    Ally ally(300.f, 300.f, textures, AllyType::Damian);
    Monster monster(350.f, 300.f);
    world.context.allEntity = {&ally, &monster};
    world.context.players = {&ally};
    world.context.allies = {&ally};
    world.context.enemies = {&monster};
    world.context.monsters = {&monster};
    ally.updateTarget(world.context);
    assert(ally.getTarget() == &monster);

    const float healthBefore = monster.getHealth();
    ally.setIsAttacking(true);
    assert(ally.getIsAttacking());
    ally.takeDamage(ally.getMaxHealth());
    assert(!ally.getIsAttacking());
    assert(!ally.canAttack());
    ally.update(world.context);
    assert(ally.getTarget() == nullptr);
    assert(!ally.getIsAttacking());
    assert(world.context.projectiles.empty());
    assertNear(monster.getHealth(), healthBefore);
}

void testA5FourConfigs(TextureManager& textures) {
    const std::array<AllyType, 4> types{{
        AllyType::Damian,
        AllyType::Evangeline,
        AllyType::Junior,
        AllyType::Lucas
    }};
    const std::array<float, 4> expectedHealth{{
        450.f, 400.f, 324.f, 360.f
    }};
    const std::array<int, 4> expectedDamage{{28, 24, 17, 38}};
    const std::array<float, 4> expectedCooldown{{
        1.75f, 2.f, 1.05f, 2.7f
    }};
    const std::array<float, 4> expectedRange{{
        250.f, 115.f, 220.f, 135.f
    }};

    for (std::size_t index = 0; index < types.size(); ++index) {
        auto ally = createAlly(
            types[index],
            {300.f + static_cast<float>(index) * 50.f, 300.f},
            textures
        );
        assert(ally->getType() == types[index]);
        assert(getAllyType(getAllyName(types[index])) == types[index]);
        assertNear(ally->getMaxHealth(), expectedHealth[index]);
        assertNear(
            ally->getAttackPower(),
            static_cast<float>(expectedDamage[index]));
        assertNear(ally->getAttackCooldown(), expectedCooldown[index]);

        const WeaponType weaponType = getAllyStats(types[index]).weaponType;
        if (weaponType == WeaponType::Sword) {
            assert(dynamic_cast<Sword*>(ally->getCurrentWeapon()));
        } else if (weaponType == WeaponType::Bow) {
            assert(dynamic_cast<Bow*>(ally->getCurrentWeapon()));
        } else {
            assert(dynamic_cast<Wand*>(ally->getCurrentWeapon()));
        }
        assertNear(ally->getAttackRange(), expectedRange[index]);
    }
}

void testA6SwordUsesAllyDamage(TextureManager& textures) {
    CombatManager combat;
    GameContext context;
    Ally attacker(300.f, 300.f, textures, AllyType::Lucas);
    Monster monster(340.f, 300.f, 500.f, 500.f);
    context.enemies = {&monster};

    const float healthBefore = monster.getHealth();
    attacker.setAttackDirection({1.f, 0.f});
    attacker.setIsAttacking(true);
    combat.processAttack(
        &attacker, attacker.getCurrentWeapon(), context.enemies);
    assertNear(
        monster.getHealth(),
        healthBefore - attacker.getAttackPower());
}

void testA7BowDamageAndFaction(TextureManager& textures) {
    TestWorld world;
    Ally archer(300.f, 300.f, textures, AllyType::Damian);
    Ally friendly(325.f, 300.f, textures, AllyType::Lucas);
    Player player(textures);
    player.setPosition({325.f, 300.f});
    Monster monster(330.f, 300.f, 500.f, 500.f);
    world.context.allEntity = {&archer, &friendly, &player, &monster};
    world.context.players = {&archer, &friendly, &player};
    world.context.allies = {&archer, &friendly};
    world.context.enemies = {&monster};
    world.context.monsters = {&monster};

    const float friendlyHealth = friendly.getHealth();
    const float playerHealth = player.getHealth();
    const float monsterHealth = monster.getHealth();
    archer.update(world.context);
    assert(world.context.projectiles.size() == 1);
    assertNear(
        world.context.projectiles.front()->getDamage(),
        archer.getAttackPower());
    world.combat.processProjectiles(
        world.context, world.context.allEntity);

    assertNear(friendly.getHealth(), friendlyHealth);
    assertNear(player.getHealth(), playerHealth);
    assertNear(
        monster.getHealth(),
        monsterHealth - archer.getAttackPower());
}

void testA8CooldownIsSingleAndIndependent(TextureManager& textures) {
    TestWorld world;
    Ally ally(300.f, 300.f, textures, AllyType::Lucas);
    Monster monster(340.f, 300.f, 1000.f, 1000.f);
    world.context.allEntity = {&ally, &monster};
    world.context.players = {&ally};
    world.context.allies = {&ally};
    world.context.enemies = {&monster};
    world.context.monsters = {&monster};

    const float initialHealth = monster.getHealth();
    ally.update(world.context);
    assertNear(
        monster.getHealth(), initialHealth - ally.getAttackPower());

    world.context.deltaTime = 0.1f;
    for (int frame = 0; frame < 10; ++frame) {
        ally.update(world.context);
    }
    const float afterFirstAttack = monster.getHealth();
    assertNear(
        afterFirstAttack, initialHealth - ally.getAttackPower());
    assert(!ally.getIsAttacking());

    world.context.deltaTime = ally.getAttackCooldown() - 0.01f;
    ally.update(world.context);
    assertNear(monster.getHealth(), afterFirstAttack);
    assert(!ally.getIsAttacking());

    world.context.deltaTime = 0.02f;
    ally.update(world.context);
    assertNear(
        monster.getHealth(), afterFirstAttack - ally.getAttackPower());
}

void testA9TwoAlliesFightIndependently(TextureManager& textures) {
    TestWorld world;
    Ally evangeline(300.f, 300.f, textures, AllyType::Evangeline);
    Ally lucas(340.f, 300.f, textures, AllyType::Lucas);
    Monster monster(380.f, 300.f, 1000.f, 1000.f);
    world.context.allEntity = {&evangeline, &lucas, &monster};
    world.context.players = {&evangeline, &lucas};
    world.context.allies = {&evangeline, &lucas};
    world.context.enemies = {&monster};
    world.context.monsters = {&monster};

    const float healthBefore = monster.getHealth();
    evangeline.update(world.context);
    assert(evangeline.getTarget() == &monster);
    assert(lucas.getTarget() == nullptr);
    lucas.update(world.context);
    assert(lucas.getTarget() == &monster);
    assertNear(
        monster.getHealth(),
        healthBefore - evangeline.getAttackPower() - lucas.getAttackPower());

    evangeline.setIsAttacking(false);
    assert(!evangeline.getIsAttacking());
    assert(lucas.getIsAttacking());
}

void testA10ProjectileOutlivesTarget(TextureManager& textures) {
    TestWorld world;
    Ally archer(300.f, 300.f, textures, AllyType::Junior);
    auto target = std::make_unique<Monster>(500.f, 300.f);
    world.context.allEntity = {&archer, target.get()};
    world.context.players = {&archer};
    world.context.allies = {&archer};
    world.context.enemies = {target.get()};
    world.context.monsters = {target.get()};

    archer.update(world.context);
    assert(world.context.projectiles.size() == 1);
    target->takeDamage(target->getMaxHealth());
    EntityLifecycle::invalidateReferencesTo(
        {target.get()}, world.context, world.combat, true);
    target.reset();
    assert(archer.getTarget() == nullptr);

    world.context.deltaTime = 0.05f;
    for (int frame = 0; frame < 5; ++frame) {
        world.combat.processProjectiles(
            world.context, world.context.allEntity);
    }
    assert(world.context.projectiles.size() == 1);
    assert(world.context.projectiles.front()->isActive());
}
}

int main() {
    TextureManager textures;
    loadTextures(textures);
    testA1SupportIsIndependent(textures);
    testA2OutsideWeaponRange(textures);
    testA3DeadTargetRetargets(textures);
    testA4DeadAllyCannotFight(textures);
    testA5FourConfigs(textures);
    testA6SwordUsesAllyDamage(textures);
    testA7BowDamageAndFaction(textures);
    testA8CooldownIsSingleAndIndependent(textures);
    testA9TwoAlliesFightIndependently(textures);
    testA10ProjectileOutlivesTarget(textures);
    return 0;
}
