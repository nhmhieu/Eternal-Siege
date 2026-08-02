#include "Boss.h"
#include "BossEnrageNotice.h"
#include "BossHealthBar.h"
#include "EnemyConfig.h"
#include "GameContext.h"
#include "GameplayTransitionGate.h"
#include "Player.h"
#include "TextureManager.h"
#include "TutorialOverlay.h"

#include <cassert>
#include <cmath>
#include <iostream>

namespace {
constexpr float EPSILON = 0.01f;

void near(float actual, float expected, float epsilon = EPSILON) {
    if (std::abs(actual - expected) > epsilon) {
        std::cerr << "near failed: actual=" << actual
                  << " expected=" << expected
                  << " epsilon=" << epsilon << '\n';
    }
    assert(std::abs(actual - expected) <= epsilon);
}

void testThresholdBoundariesAndSingleActivation() {
    Boss above(300.f, 300.f, 4);
    above.takeDamage(above.getMaxHealth() * 0.749f);
    assert(!above.isEnraged());
    assert(above.getEnrageActivationCount() == 0);

    Boss exact(300.f, 300.f, 4);
    exact.takeDamage(exact.getMaxHealth() * 0.75f);
    assert(exact.isEnraged());
    assert(exact.getPhase() == Boss::Phase::Enraged);
    assert(exact.getEnrageActivationCount() == 1);

    Boss below(300.f, 300.f, 4);
    below.takeDamage(below.getMaxHealth() * 0.76f);
    assert(!below.isDead());
    assert(below.isEnraged());
    assert(below.getEnrageActivationCount() == 1);

    GameContext context;
    context.deltaTime = 0.1f;
    for (int frame = 0; frame < 20; ++frame) below.update(context);
    assert(below.getEnrageActivationCount() == 1);
}

void testEnragedSpeedAndDamageResistanceAppliedExactlyOnce() {
    Boss boss(300.f, 300.f, 4);
    const float baseSpeed = boss.getMoveSpeed();
    const float maxHealth = boss.getMaxHealth();

    const float crossingDamage = maxHealth * 0.76f;
    boss.takeDamage(crossingDamage);
    near(boss.getHealth(), maxHealth - crossingDamage);
    near(boss.getMoveSpeed(),
         baseSpeed * Boss::ENRAGED_SPEED_MULTIPLIER);

    GameContext context;
    context.deltaTime = 0.2f;
    for (int frame = 0; frame < 10; ++frame) boss.update(context);
    near(boss.getMoveSpeed(),
         baseSpeed * Boss::ENRAGED_SPEED_MULTIPLIER);

    const float before = boss.getHealth();
    boss.takeDamage(100.f);
    near(boss.getHealth(),
         before - 100.f * Boss::ENRAGED_DAMAGE_TAKEN_MULTIPLIER);
}

void testLethalCrossingDoesNotEnrageAndStopsBeam() {
    Boss boss(300.f, 300.f, 4);
    boss.takeDamage(boss.getMaxHealth() * 0.5f);
    assert(boss.getBeamState() == Boss::BeamState::Warning);

    boss.takeDamage(boss.getMaxHealth() * 2.f);
    assert(boss.isDead());
    assert(!boss.isEnraged());
    assert(boss.getEnrageActivationCount() == 0);
    assert(boss.getBeamState() == Boss::BeamState::Inactive);
    near(boss.getBeamTimer(), 0.f);
}

void testBeamPhaseAndTimerRemainIndependent() {
    Boss boss(300.f, 300.f, 4);
    boss.takeDamage(boss.getMaxHealth() * 0.5f);
    assert(boss.getPhase() == Boss::Phase::Phase2);
    assert(boss.getPhaseTwoActivationCount() == 1);
    assert(boss.getBeamVolleyCount() == 1);

    GameContext context;
    context.deltaTime = 0.2f;
    boss.update(context);
    const Boss::BeamState stateBefore = boss.getBeamState();
    const float timerBefore = boss.getBeamTimer();
    const int volleysBefore = boss.getBeamVolleyCount();

    boss.takeDamage(boss.getMaxHealth() * 0.25f);
    assert(boss.isEnraged());
    assert(boss.getBeamState() == stateBefore);
    near(boss.getBeamTimer(), timerBefore);
    assert(boss.getBeamVolleyCount() == volleysBefore);
    assert(boss.getPhaseTwoActivationCount() == 1);
}

void testBeamHitsPlayerOnceAndPauseFreezes() {
    TextureManager textures;
    Player player(textures);
    player.setPosition({500.f, 300.f});
    Boss boss(300.f, 300.f, 4);
    boss.takeDamage(boss.getMaxHealth() * 0.5f);

    GameContext context;
    context.players = {&player};
    context.paused = true;
    context.deltaTime = 0.4f;
    const float frozenTimer = boss.getBeamTimer();
    boss.update(context);
    near(boss.getBeamTimer(), frozenTimer);

    context.paused = false;
    context.deltaTime = Boss::BEAM_WARNING_DURATION;
    const float before = player.getHealth();
    boss.update(context);
    assert(boss.getBeamState() == Boss::BeamState::Active);
    context.deltaTime = 0.01f;
    boss.update(context);
    near(player.getHealth(), before - Boss::BEAM_DAMAGE);
    context.deltaTime = 0.1f;
    boss.update(context);
    near(player.getHealth(), before - Boss::BEAM_DAMAGE);
}

void testNoticeIsOneShotPausedAndResettable() {
    BossEnrageNotice notice;
    notice.resetForNewGame();
    assert(!notice.observe(false, false));
    assert(!notice.observe(true, false));
    assert(notice.observe(true, true));
    assert(notice.isVisible());
    assert(notice.getActivationCount() == 1);
    assert(!notice.observe(true, true));

    const float beforePause = notice.getRemaining();
    notice.update(1.f, true);
    near(notice.getRemaining(), beforePause);
    notice.update(0.5f, false);
    near(notice.getRemaining(), beforePause - 0.5f);

    // Tutorial blocks GameplayState::update, so observe/update are not called.
    const float beforeTutorialBlock = notice.getRemaining();
    near(notice.getRemaining(), beforeTutorialBlock);
    assert(!notice.observe(true, true));
    assert(notice.getActivationCount() == 1);

    notice.resetForNewGame();
    assert(!notice.hasBeenShown());
    assert(notice.getActivationCount() == 0);
    assert(notice.observe(true, true));
    assert(notice.getActivationCount() == 1);
}

void testTutorialOpenCloseDoesNotRepeatEnrageTransition() {
    TutorialController tutorial;
    BossEnrageNotice notice;
    tutorial.resetForNewGame();
    notice.resetForNewGame();

    assert(tutorial.handleKeyPressed(TutorialKey::Enter) ==
           TutorialTransition::InitialClosed);
    tutorial.handleKeyReleased(TutorialKey::Enter);
    tutorial.finishInputRelease();
    assert(notice.observe(true, true));
    assert(notice.getActivationCount() == 1);

    assert(tutorial.handleKeyPressed(TutorialKey::Help) ==
           TutorialTransition::Reopened);
    assert(tutorial.blocksGameplayUpdate());
    tutorial.handleKeyReleased(TutorialKey::Help);
    assert(!notice.observe(true, true));

    assert(tutorial.handleKeyPressed(TutorialKey::Help) ==
           TutorialTransition::Closed);
    tutorial.handleKeyReleased(TutorialKey::Help);
    tutorial.finishInputRelease();
    assert(!tutorial.blocksGameplayUpdate());
    assert(!notice.observe(true, true));
    assert(notice.getActivationCount() == 1);
}

void testVictoryAndGameOverTransitionsAreAcceptedOnce() {
    GameplayTransitionGate gate;
    gate.reset();
    assert(gate.request(GameplayEndState::Victory));
    assert(!gate.request(GameplayEndState::Victory));
    assert(!gate.request(GameplayEndState::GameOver));
    assert(gate.getRequestedState() == GameplayEndState::Victory);
    assert(gate.getAcceptedCount() == 1);

    gate.reset();
    // GameplayState checks Player death first, preserving Game Over priority.
    assert(gate.request(GameplayEndState::GameOver));
    assert(!gate.request(GameplayEndState::Victory));
    assert(gate.getRequestedState() == GameplayEndState::GameOver);
    assert(gate.getAcceptedCount() == 1);
}

void testHealthBarSnapshotMarkerAndEnragedState() {
    near(Boss::getEnrageThresholdRatio(), 0.25f);
    near(BossHealthBar::getEnrageMarkerRatio(),
         Boss::getEnrageThresholdRatio());

    BossHealthBar bar;
    Boss boss(300.f, 300.f, 4);
    bar.update(&boss, 0.1f, false);
    assert(bar.isVisible());
    assert(!bar.isDisplayedEnraged());
    boss.takeDamage(boss.getMaxHealth() * 0.75f);
    bar.update(&boss, 0.1f, false);
    assert(bar.isDisplayedEnraged());
    near(bar.getDisplayedHealth(), boss.getHealth());

    boss.takeDamage(100000.f);
    bar.update(&boss, 0.1f, false);
    assert(!bar.isVisible());
    assert(!bar.isDisplayedEnraged());
    bar.update(nullptr, 0.1f, false);
    assert(!bar.isVisible());
}

void testBossBalanceUnchanged() {
    const EnemyConfig::Stats stats = EnemyConfig::bossStats(4);
    near(stats.maxHealth, 1020.f);
    near(stats.attackRange, 62.f);
    near(stats.cooldown, 0.8f);
    near(stats.speed, 62.f);
    assert(stats.effectiveDamage == 28);
    near(Boss::BEAM_DAMAGE, 22.f);
    near(Boss::BEAM_WARNING_DURATION, 0.80f);
    near(Boss::BEAM_ACTIVE_DURATION, 0.35f);
    near(Boss::BEAM_COOLDOWN_DURATION, 4.00f);
}

void testBossReward150GoldClaimedExactlyOnce() {
    Boss boss(300.f, 300.f, 4);
    boss.takeDamage(100000.f);
    assert(boss.claimGoldReward() == 150);
    assert(boss.claimGoldReward() == 0);
}
}

int main() {
    testThresholdBoundariesAndSingleActivation();
    testEnragedSpeedAndDamageResistanceAppliedExactlyOnce();
    testLethalCrossingDoesNotEnrageAndStopsBeam();
    testBeamPhaseAndTimerRemainIndependent();
    testBeamHitsPlayerOnceAndPauseFreezes();
    testNoticeIsOneShotPausedAndResettable();
    testTutorialOpenCloseDoesNotRepeatEnrageTransition();
    testVictoryAndGameOverTransitionsAreAcceptedOnce();
    testHealthBarSnapshotMarkerAndEnragedState();
    testBossBalanceUnchanged();
    testBossReward150GoldClaimedExactlyOnce();
    std::cout << "Boss Enraged tests passed\n";
    return 0;
}
