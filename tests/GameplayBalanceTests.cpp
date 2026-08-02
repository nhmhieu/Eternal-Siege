#include "Ally.h"
#include "Arrow.h"
#include "Boss.h"
#include "Bow.h"
#include "CombatManager.h"
#include "EntityLifecycle.h"
#include "Effects.h"
#include "Elite.h"
#include "GameContext.h"
#include "Map.h"
#include "MagicBolt.h"
#include "Player.h"
#include "Sword.h"
#include "TextureManager.h"
#include "WaveManager.h"
#include "Wand.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>

namespace {
constexpr float EPSILON = 0.001f;

void assertNear(float actual, float expected) {
    if (std::abs(actual - expected) > EPSILON) {
        std::cerr << "assertNear actual=" << actual
                  << " expected=" << expected << '\n';
    }
    assert(std::abs(actual - expected) <= EPSILON);
}

void enterPhaseTwo(Boss& boss) {
    boss.takeDamage(boss.getMaxHealth() * 0.5f);
    assert(boss.getPhase() == Boss::Phase::Phase2);
}

void testAllyBalance(TextureManager& textures) {
    Ally junior(0.f, 0.f, textures, AllyType::Junior);
    assertNear(junior.getMaxHealth(), 324.f);
    assertNear(junior.getAttackPower(), 17.f);
    assertNear(junior.getAttackCooldown(), 1.05f);
    assertNear(junior.getAttackRange(), 220.f);
    assert(dynamic_cast<Wand*>(junior.getCurrentWeapon()));

    Ally damian(0.f, 0.f, textures, AllyType::Damian);
    assertNear(damian.getMaxHealth(), 450.f);
    assertNear(damian.getAttackPower(), 28.f);
    assertNear(damian.getAttackCooldown(), 1.75f);
    assertNear(damian.getAttackRange(), 250.f);
    assert(dynamic_cast<Bow*>(damian.getCurrentWeapon()));

    Ally evangeline(0.f, 0.f, textures, AllyType::Evangeline);
    assertNear(evangeline.getMaxHealth(), 400.f);
    assertNear(evangeline.getAttackPower(), 24.f);
    assertNear(evangeline.getAttackCooldown(), 2.f);
    assertNear(evangeline.getAttackRange(), 115.f);
    assert(dynamic_cast<Sword*>(evangeline.getCurrentWeapon()));
    const float evangelineHealth = evangeline.getHealth();
    evangeline.takeDamage(100.f);
    assertNear(evangeline.getHealth(), evangelineHealth - 75.f);

    Ally lucas(0.f, 0.f, textures, AllyType::Lucas);
    assertNear(lucas.getMaxHealth(), 360.f);
    assertNear(lucas.getAttackPower(), 38.f);
    assertNear(lucas.getAttackCooldown(), 2.7f);
    assertNear(lucas.getAttackRange(), 135.f);
    assert(dynamic_cast<Sword*>(lucas.getCurrentWeapon()));
}

void testOfficialWeaponProjectiles(TextureManager& textures) {
    Map map(15, 15);
    CombatManager combat;
    GameContext context;
    Effects effects;
    context.map = &map;
    context.combatManager = &combat;
    context.effects = &effects;
    context.deltaTime = 0.01f;

    Ally junior(300.f, 300.f, textures, AllyType::Junior);
    Ally friendly(315.f, 300.f, textures, AllyType::Evangeline);
    Monster magicTarget(340.f, 300.f, 500.f, 500.f);
    context.enemies = {&magicTarget};
    context.allEntity = {&junior, &friendly, &magicTarget};
    const float friendlyHealth = friendly.getHealth();
    junior.update(context);
    assert(context.projectiles.size() == 1);
    assert(dynamic_cast<MagicBolt*>(context.projectiles.front().get()));
    assert(!dynamic_cast<Arrow*>(context.projectiles.front().get()));
    assert(context.projectiles.front()->getStyle() == ProjectileStyle::Magic);
    assertNear(context.projectiles.front()->getDamage(), 17.f);
    const std::size_t magicEffects = effects.activeEffectCount();
    assert(magicEffects > 0);
    junior.update(context);
    assert(effects.activeEffectCount() == magicEffects);
    const sf::Vector2f pausedPosition =
        context.projectiles.front()->getPosition();
    const std::size_t pausedTrail =
        context.projectiles.front()->getTrailPointCount();
    const float pausedVisualTime = junior.getVisualTime();
    context.paused = true;
    junior.update(context);
    context.projectiles.front()->update(context);
    assertNear(junior.getVisualTime(), pausedVisualTime);
    assert(effects.activeEffectCount() == magicEffects);
    assert(context.projectiles.front()->getPosition() == pausedPosition);
    assert(context.projectiles.front()->getTrailPointCount() == pausedTrail);
    context.paused = false;
    junior.setAttackPower(99.f);
    assertNear(context.projectiles.front()->getDamage(), 17.f);
    context.deltaTime = 0.06f;
    combat.processProjectiles(context, context.allEntity);
    assertNear(friendly.getHealth(), friendlyHealth);
    assertNear(magicTarget.getHealth(), 483.f);

    context.projectiles.clear();
    Ally damian(300.f, 300.f, textures, AllyType::Damian);
    Monster arrowTarget(340.f, 300.f, 500.f, 500.f);
    context.enemies = {&arrowTarget};
    context.allEntity = {&damian, &friendly, &arrowTarget};
    damian.update(context);
    assert(context.projectiles.size() == 1);
    assert(dynamic_cast<Arrow*>(context.projectiles.front().get()));
    assert(!dynamic_cast<MagicBolt*>(context.projectiles.front().get()));
    assert(context.projectiles.front()->getStyle() == ProjectileStyle::Arrow);
    assertNear(context.projectiles.front()->getDamage(), 28.f);

    context.projectiles.clear();
    Ally evangeline(300.f, 300.f, textures, AllyType::Evangeline);
    Monster swordTarget(340.f, 300.f, 500.f, 500.f);
    context.enemies = {&swordTarget};
    evangeline.update(context);
    assert(context.projectiles.empty());
    assertNear(swordTarget.getHealth(), 476.f);

    Ally lucas(300.f, 300.f, textures, AllyType::Lucas);
    Monster lucasTarget(340.f, 300.f, 500.f, 500.f);
    context.enemies = {&lucasTarget};
    lucas.update(context);
    assert(context.projectiles.empty());
    assertNear(lucasTarget.getHealth(), 462.f);
}

void testMonsterPresentationSizing(TextureManager& textures) {
    assert(textures.loadTexture(
        "MonsterNormal", "assets/images/monsters/NormalMonster.png"));
    assert(textures.loadTexture(
        "MonsterElite", "assets/images/monsters/EliteMonster.png"));
    assert(textures.loadTexture(
        "MonsterBoss", "assets/images/monsters/Boss.png"));

    Monster normal(300.f, 300.f);
    const sf::Vector2f normalCollision = normal.getCollisionSize();
    normal.setPresentationTexture(
        textures.findTexture("MonsterNormal"),
        textures.getVisibleBounds("MonsterNormal"), 58.f);
    assertNear(normal.getVisualSize().y, 58.f);
    assert(normal.getCollisionSize() == normalCollision);

    Elite elite(300.f, 300.f, 4);
    const sf::Vector2f eliteCollision = elite.getCollisionSize();
    elite.setPresentationTexture(
        textures.findTexture("MonsterElite"),
        textures.getVisibleBounds("MonsterElite"), 72.f);
    assertNear(elite.getVisualSize().y, 72.f);
    assert(elite.getCollisionSize() == eliteCollision);

    Boss boss(300.f, 300.f, 4);
    const sf::Vector2f bossCollision = boss.getCollisionSize();
    const sf::Vector2f bossHurtBox = boss.getHurtBoxSize();
    boss.setPresentationTexture(
        textures.findTexture("MonsterBoss"),
        textures.getVisibleBounds("MonsterBoss"), 122.f);
    assertNear(boss.getVisualSize().y, 122.f);
    assert(boss.getVisualSize().y > elite.getVisualSize().y);
    assert(boss.getCollisionSize() == bossCollision);
    assert(boss.getHurtBoxSize() == bossHurtBox);
}

void testMagicBoltOutlivesTarget(TextureManager& textures) {
    Map map(15, 15);
    CombatManager combat;
    GameContext context;
    context.map = &map;
    context.combatManager = &combat;
    context.deltaTime = 0.01f;
    Ally junior(300.f, 300.f, textures, AllyType::Junior);
    auto target = std::make_unique<Monster>(500.f, 300.f);
    context.allEntity = {&junior, target.get()};
    context.allies = {&junior};
    context.monsters = {target.get()};
    context.enemies = {target.get()};
    junior.update(context);
    assert(context.projectiles.size() == 1);
    assert(dynamic_cast<MagicBolt*>(context.projectiles.front().get()));
    EntityLifecycle::invalidateReferencesTo(
        {target.get()}, context, combat, true);
    target.reset();
    context.deltaTime = 0.05f;
    for (int frame = 0; frame < 5; ++frame) {
        combat.processProjectiles(context, context.allEntity);
    }
    assert(context.projectiles.size() == 1);
    assert(context.projectiles.front()->isActive());
}

void testBossPhaseTwoAndWarning(TextureManager& textures) {
    Boss boss(300.f, 300.f, 4);
    Player player(textures);
    player.setPosition({500.f, 300.f});
    GameContext context;
    context.players = {&player};

    context.deltaTime = 1.f;
    boss.update(context);
    assert(boss.getPhase() == Boss::Phase::Phase1);
    assert(boss.getBeamState() == Boss::BeamState::Inactive);
    assertNear(player.getHealth(), player.getMaxHealth());

    enterPhaseTwo(boss);
    assert(boss.getPhaseTwoActivationCount() == 1);
    assert(boss.getBeamVolleyCount() == 1);
    boss.takeDamage(10.f);
    boss.update(context);
    assert(boss.getPhaseTwoActivationCount() == 1);

    Boss warningBoss(300.f, 300.f, 4);
    enterPhaseTwo(warningBoss);
    const float before = player.getHealth();
    context.deltaTime = Boss::BEAM_WARNING_DURATION - 0.01f;
    warningBoss.update(context);
    assert(warningBoss.getBeamState() == Boss::BeamState::Warning);
    assertNear(player.getHealth(), before);
    context.deltaTime = 0.01f;
    warningBoss.update(context);
    assert(warningBoss.getBeamState() == Boss::BeamState::Active);
    assertNear(player.getHealth(), before - Boss::BEAM_DAMAGE);
    context.deltaTime = 0.01f;
    warningBoss.update(context);
    assertNear(player.getHealth(), before - Boss::BEAM_DAMAGE);
    context.deltaTime = 0.10f;
    warningBoss.update(context);
    assertNear(player.getHealth(), before - Boss::BEAM_DAMAGE);
}

void testBeamTargetsOnlyPlayerAndRespectsCooldown(TextureManager& textures) {
    Boss boss(300.f, 300.f, 4);
    Player player(textures);
    player.setPosition({500.f, 300.f});
    Ally ally(450.f, 300.f, textures, AllyType::Damian);
    GameContext context;
    context.players = {&ally, &player};
    enterPhaseTwo(boss);

    const float allyHealth = ally.getHealth();
    context.deltaTime = Boss::BEAM_WARNING_DURATION;
    boss.update(context);
    context.deltaTime = 0.01f;
    boss.update(context);
    assertNear(ally.getHealth(), allyHealth);
    assertNear(player.getHealth(), player.getMaxHealth() - Boss::BEAM_DAMAGE);

    context.deltaTime = Boss::BEAM_ACTIVE_DURATION - 0.01f;
    boss.update(context);
    assert(boss.getBeamState() == Boss::BeamState::Cooldown);
    assert(boss.getBeamVolleyCount() == 1);
    context.deltaTime = Boss::BEAM_COOLDOWN_DURATION - 0.01f;
    boss.update(context);
    assert(boss.getBeamVolleyCount() == 1);
    context.deltaTime = 0.01f;
    boss.update(context);
    assert(boss.getBeamState() == Boss::BeamState::Warning);
    assert(boss.getBeamVolleyCount() == 2);
}

void testBossPhaseThreeIsIdempotent() {
    Boss boss(300.f, 300.f, 4);
    const float baseSpeed = boss.getMoveSpeed();
    boss.takeDamage(boss.getMaxHealth() * 0.8f);
    assert(boss.getPhase() == Boss::Phase::Enraged);
    assert(boss.getPhaseTwoActivationCount() == 1);
    assert(boss.getEnrageActivationCount() == 1);
    assertNear(boss.getMoveSpeed(),
               baseSpeed * Boss::ENRAGED_SPEED_MULTIPLIER);

    const float before = boss.getHealth();
    boss.takeDamage(100.f);
    assertNear(boss.getHealth(),
               before - 100.f * Boss::ENRAGED_DAMAGE_TAKEN_MULTIPLIER);
    GameContext context;
    context.deltaTime = 1.f;
    boss.update(context);
    assert(boss.getEnrageActivationCount() == 1);
    assertNear(boss.getMoveSpeed(),
               baseSpeed * Boss::ENRAGED_SPEED_MULTIPLIER);
}

void testPauseFreezesEveryBeamTimer(TextureManager& textures) {
    Boss boss(300.f, 300.f, 4);
    Player player(textures);
    player.setPosition({500.f, 300.f});
    GameContext context;
    context.players = {&player};
    enterPhaseTwo(boss);

    context.paused = true;
    context.deltaTime = 0.4f;
    const float warningTimer = boss.getBeamTimer();
    boss.update(context);
    assertNear(boss.getBeamTimer(), warningTimer);

    context.paused = false;
    context.deltaTime = Boss::BEAM_WARNING_DURATION;
    boss.update(context);
    assert(boss.getBeamState() == Boss::BeamState::Active);
    context.paused = true;
    context.deltaTime = 0.2f;
    const float activeTimer = boss.getBeamTimer();
    const float playerHealth = player.getHealth();
    boss.update(context);
    assertNear(boss.getBeamTimer(), activeTimer);
    assertNear(player.getHealth(), playerHealth);

    context.paused = false;
    context.deltaTime = Boss::BEAM_ACTIVE_DURATION;
    boss.update(context);
    assert(boss.getBeamState() == Boss::BeamState::Cooldown);
    context.paused = true;
    context.deltaTime = 2.f;
    const float cooldownTimer = boss.getBeamTimer();
    boss.update(context);
    assertNear(boss.getBeamTimer(), cooldownTimer);
}

void testBossDeathCompletesFourWavePlan() {
    Map map(15, 15);
    WaveManager waves;
    bool bossDefeated = false;
    for (int guard = 0; guard < 500 && !waves.isGameCompleted(); ++guard) {
        if (waves.isIntermission()) waves.startNextWave();
        auto spawned = waves.update(10.f, map, true);
        if (spawned && spawned->isBoss()) {
            spawned->takeDamage(100000.f);
            assert(spawned->isDead());
            bossDefeated = true;
        }
    }
    assert(bossDefeated);
    assert(waves.isGameCompleted());
}
}

int main() {
    TextureManager textures;
    testAllyBalance(textures);
    testOfficialWeaponProjectiles(textures);
    testMonsterPresentationSizing(textures);
    testMagicBoltOutlivesTarget(textures);
    testBossPhaseTwoAndWarning(textures);
    testBeamTargetsOnlyPlayerAndRespectsCooldown(textures);
    testBossPhaseThreeIsIdempotent();
    testPauseFreezesEveryBeamTimer(textures);
    testBossDeathCompletesFourWavePlan();
    std::cout << "Gameplay Balance v2 tests passed\n";
    return 0;
}
