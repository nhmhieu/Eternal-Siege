#include "AssetLocator.h"
#include "Ally.h"
#include "Boss.h"
#include "BossEnrageNotice.h"
#include "BossHealthBar.h"
#include "Constants.h"
#include "Effects.h"
#include "EnemyConfig.h"
#include "GameContext.h"
#include "GameplayTransitionGate.h"
#include "IntermissionOverlay.h"
#include "Map.h"
#include "Player.h"
#include "SpiritBolt.h"
#include "TextureManager.h"
#include "TutorialOverlay.h"
#include "UpgradeManager.h"
#include "WaveManager.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

namespace {
constexpr float EPSILON = 0.01f;

void near(float actual, float expected) {
    assert(std::abs(actual - expected) <= EPSILON);
}

void testProductionInputPolicyUsesPressedEdges() {
    static_assert(!GameConfig::KEY_REPEAT_ENABLED,
                  "Runtime key repeat must remain disabled");
}

void testTransitionGateAcrossThreeSessions() {
    GameplayTransitionGate sessionA;
    sessionA.reset();
    assert(sessionA.request(GameplayEndState::Victory));
    assert(!sessionA.request(GameplayEndState::Victory));
    assert(!sessionA.request(GameplayEndState::GameOver));
    assert(sessionA.getAcceptedCount() == 1);

    GameplayTransitionGate sessionB;
    sessionB.reset();
    assert(sessionB.request(GameplayEndState::GameOver));
    assert(!sessionB.request(GameplayEndState::GameOver));
    assert(!sessionB.request(GameplayEndState::Victory));
    assert(sessionB.getRequestedState() == GameplayEndState::GameOver);
    assert(sessionB.getAcceptedCount() == 1);

    GameplayTransitionGate sessionC;
    sessionC.reset();
    assert(!sessionC.hasRequest());
    assert(sessionC.request(GameplayEndState::Victory));
    assert(sessionC.getAcceptedCount() == 1);
}

void testBossRewardAndEnrageResetPerSession() {
    for (int session = 0; session < 2; ++session) {
        Boss boss(300.f, 300.f, 4);
        BossEnrageNotice notice;
        notice.resetForNewGame();

        boss.takeDamage(boss.getMaxHealth() * 0.75f);
        assert(boss.isEnraged());
        assert(notice.observe(true, boss.isEnraged()));
        assert(!notice.observe(true, boss.isEnraged()));
        assert(notice.getActivationCount() == 1);

        boss.takeDamage(100000.f);
        assert(boss.isDead());
        assert(boss.getBeamState() == Boss::BeamState::Inactive);
        assert(boss.claimGoldReward() == 150);
        assert(boss.claimGoldReward() == 0);
    }
}

void testBossDeathCompletesWaveFourWithoutIntermission() {
    Map map(15, 15);
    WaveManager waves;
    GameplayTransitionGate transition;
    bool bossDefeated = false;

    for (int guard = 0; guard < 600 && !waves.isGameCompleted(); ++guard) {
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
    assert(!waves.isIntermission());
    assert(transition.request(GameplayEndState::Victory));
    assert(!transition.request(GameplayEndState::Victory));
    assert(transition.getAcceptedCount() == 1);

    waves.startNextWave();
    assert(waves.isGameCompleted());
    assert(waves.getCurrentWave() == 4);
}

void testTutorialAndIntermissionInputEdges() {
    TutorialController tutorial;
    tutorial.resetForNewGame();
    assert(tutorial.handleKeyPressed(TutorialKey::Enter) ==
           TutorialTransition::InitialClosed);
    assert(tutorial.handleKeyPressed(TutorialKey::Enter) ==
           TutorialTransition::Consumed);
    tutorial.handleKeyReleased(TutorialKey::Enter);
    tutorial.finishInputRelease();

    BossEnrageNotice notice;
    notice.resetForNewGame();
    assert(notice.observe(true, true));
    assert(tutorial.handleKeyPressed(TutorialKey::Help) ==
           TutorialTransition::Reopened);
    assert(tutorial.blocksGameplayUpdate());
    tutorial.handleKeyReleased(TutorialKey::Help);
    assert(!notice.observe(true, true));
    assert(tutorial.handleKeyPressed(TutorialKey::Help) ==
           TutorialTransition::Closed);
    tutorial.handleKeyReleased(TutorialKey::Help);
    tutorial.finishInputRelease();
    assert(!notice.observe(true, true));
    assert(notice.getActivationCount() == 1);

    IntermissionController intermission;
    intermission.resetForNewGame();
    intermission.sync(true, 1);
    assert(intermission.handleEnter(0) ==
           IntermissionAction::ShowSkipWarning);
    assert(intermission.handleEnter(0) == IntermissionAction::Consumed);
    intermission.handleEnterReleased();
    assert(intermission.handleEnter(0) ==
           IntermissionAction::BeginNextWave);
}

void testWaveStartLocksUndoHistoryAndRunsOnce() {
    Map map(15, 15);
    WaveManager waves;
    auto spawned = waves.update(10.f, map, true);
    assert(spawned != nullptr);
    waves.update(0.f, map, true);
    assert(waves.isIntermission());

    TextureManager textures;
    Player player(textures);
    std::vector<std::unique_ptr<Ally>> allies;
    UpgradeManager upgrades;
    assert(upgrades.purchase(UpgradeType::Damage, player, allies));
    assert(upgrades.getPendingPurchaseCount() == 1);

    waves.startNextWave();
    upgrades.clearUndoHistory();
    assert(waves.getCurrentWave() == 2);
    assert(waves.isWaveActive());
    assert(!upgrades.canUndo());

    waves.startNextWave();
    assert(waves.getCurrentWave() == 2);
}

void testSessionOwnedProjectileEffectsAndBossBarReset() {
    GameContext oldContext;
    oldContext.projectiles.push_back(std::make_unique<SpiritBolt>(
        sf::Vector2f{10.f, 10.f}, sf::Vector2f{1.f, 0.f},
        300.f, 18.f, Team::Player));
    assert(oldContext.projectiles.size() == 1);

    Effects effects;
    effects.spawnPortal({100.f, 100.f}, true);
    assert(effects.activeEffectCount() > 0);
    effects.clear();
    assert(effects.activeEffectCount() == 0);

    GameContext newContext;
    assert(newContext.projectiles.empty());

    BossHealthBar bar;
    auto boss = std::make_unique<Boss>(300.f, 300.f, 4);
    bar.update(boss.get(), 0.1f, false);
    assert(bar.isVisible());
    near(bar.getDisplayedHealth(), boss->getHealth());
    boss->takeDamage(100000.f);
    bar.update(boss.get(), 0.1f, false);
    assert(!bar.isVisible());
    boss.reset();
    bar.update(nullptr, 0.1f, false);
    assert(!bar.isVisible());
}

void testFrozenBalanceAndAssetResolution() {
    const EnemyConfig::Stats boss = EnemyConfig::bossStats(4);
    near(boss.maxHealth, 1020.f);
    near(boss.attackRange, 62.f);
    near(boss.cooldown, 0.8f);
    near(boss.speed, 62.f);
    assert(boss.effectiveDamage == 28);
    near(Boss::PHASE_TWO_HEALTH_RATIO, 0.50f);
    near(Boss::getEnrageThresholdRatio(), 0.25f);
    near(Boss::ENRAGED_SPEED_MULTIPLIER, 1.45f);
    near(Boss::ENRAGED_DAMAGE_TAKEN_MULTIPLIER, 0.65f);
    near(Boss::BEAM_DAMAGE, 22.f);

    assert(AssetLocator::find("assets/fonts/Font.ttf").has_value());
    assert(AssetLocator::find(
        "assets/images/PlayerMage.png").has_value());
    assert(AssetLocator::find(
        "assets/audio/music/gameplay_theme.ogg").has_value());
}
}

int main() {
    testProductionInputPolicyUsesPressedEdges();
    testTransitionGateAcrossThreeSessions();
    testBossRewardAndEnrageResetPerSession();
    testBossDeathCompletesWaveFourWithoutIntermission();
    testTutorialAndIntermissionInputEdges();
    testWaveStartLocksUndoHistoryAndRunsOnce();
    testSessionOwnedProjectileEffectsAndBossBarReset();
    testFrozenBalanceAndAssetResolution();
    std::cout << "Full-game integration tests passed\n";
    return 0;
}
