#include "Ally.h"
#include "BalanceConfig.h"
#include "IntermissionOverlay.h"
#include "Map.h"
#include "Player.h"
#include "TextureManager.h"
#include "TutorialOverlay.h"
#include "UpgradeManager.h"
#include "WaveManager.h"

#include <array>
#include <cassert>
#include <cmath>
#include <memory>
#include <string_view>
#include <vector>

namespace {
constexpr float EPSILON = 0.0001f;
void assertNear(float actual, float expected) {
    assert(std::abs(actual - expected) <= EPSILON);
}

std::vector<std::unique_ptr<Ally>> makeAllies(TextureManager& textures) {
    std::vector<std::unique_ptr<Ally>> allies;
    for (const AllyType type : {AllyType::Damian, AllyType::Evangeline,
                                AllyType::Junior, AllyType::Lucas}) {
        allies.push_back(createAlly(type, {200.f, 200.f}, textures));
    }
    return allies;
}

void completeWaveOne(WaveManager& waves, const Map& map) {
    assert(waves.update(0.8f, map, true) != nullptr);
    assert(!waves.isIntermission());
    assert(waves.update(0.f, map, true) == nullptr);
    assert(waves.isIntermission());
    assert(waves.getCurrentWave() == 1);
}

void testPreviewHasNoSideEffects() {
    UpgradeManager upgrades;
    const int gold = upgrades.getGold();
    for (const UpgradeType type : {UpgradeType::Damage,
                                   UpgradeType::Vitality,
                                   UpgradeType::FireRate}) {
        const int level = upgrades.getLevel(type);
        const UpgradePreview first = upgrades.preview(type);
        const UpgradePreview second = upgrades.preview(type);
        assert(first.level == level);
        assert(first.nextLevel == level + 1);
        assert(first.cost == 20);
        assert(first.affordable);
        assert(!first.maxLevel);
        assert(std::string_view(first.name) == std::string_view(second.name));
        assert(upgrades.getLevel(type) == level);
    }
    assert(upgrades.getGold() == gold);
    assert(upgrades.getPendingPurchaseCount() == 0);
}

void testUpgradeFormulasAndFailure(TextureManager& textures) {
    Player player(textures);
    player.setAttackPower(BalanceConfig::PLAYER_DAMAGE);
    player.setAttackCooldown(0.55f);
    auto allies = makeAllies(textures);
    UpgradeManager upgrades;

    const float playerDamage = player.getAttackPower();
    const float allyDamage = allies.front()->getAttackPower();
    assert(upgrades.purchase(UpgradeType::Damage, player, allies));
    assertNear(player.getAttackPower(), playerDamage + 4.f);
    assertNear(allies.front()->getAttackPower(), allyDamage + 3.f);
    assert(upgrades.getGold() == 5);
    assert(upgrades.getPendingPurchaseCount() == 1);

    const float health = player.getMaxHealth();
    const int gold = upgrades.getGold();
    assert(!upgrades.purchase(UpgradeType::Vitality, player, allies));
    assertNear(player.getMaxHealth(), health);
    assert(upgrades.getGold() == gold);
    assert(upgrades.getPendingPurchaseCount() == 1);

    Player vitalityPlayer(textures);
    auto vitalityAllies = makeAllies(textures);
    UpgradeManager vitality;
    const float playerHealth = vitalityPlayer.getMaxHealth();
    const float allyHealth = vitalityAllies.front()->getMaxHealth();
    assert(vitality.purchase(
        UpgradeType::Vitality, vitalityPlayer, vitalityAllies));
    assertNear(vitalityPlayer.getMaxHealth(), playerHealth + 25.f);
    assertNear(vitalityAllies.front()->getMaxHealth(), allyHealth + 15.f);

    Player ratePlayer(textures);
    ratePlayer.setAttackCooldown(0.55f);
    auto rateAllies = makeAllies(textures);
    UpgradeManager rate;
    const float playerCooldown = ratePlayer.getAttackCooldown();
    const float allyCooldown = rateAllies.front()->getAttackCooldown();
    assert(rate.purchase(UpgradeType::FireRate, ratePlayer, rateAllies));
    assertNear(ratePlayer.getAttackCooldown(), playerCooldown * 0.84f);
    assertNear(rateAllies.front()->getAttackCooldown(), allyCooldown * 0.88f);
}

void testUndoLifoAndRetainedCount(TextureManager& textures) {
    Player player(textures);
    player.setAttackPower(BalanceConfig::PLAYER_DAMAGE);
    player.setAttackCooldown(0.55f);
    auto allies = makeAllies(textures);
    UpgradeManager upgrades;
    upgrades.addGold(100);
    const int initialGold = upgrades.getGold();

    assert(upgrades.purchase(UpgradeType::Damage, player, allies));
    assert(upgrades.purchase(UpgradeType::Vitality, player, allies));
    assert(upgrades.purchase(UpgradeType::FireRate, player, allies));
    assert(upgrades.getPendingPurchaseCount() == 3);
    assert(upgrades.getLevel(UpgradeType::FireRate) == 1);
    assert(upgrades.undoLastPurchase(player, allies));
    assert(upgrades.getLevel(UpgradeType::FireRate) == 0);
    assert(upgrades.getLevel(UpgradeType::Vitality) == 1);
    assert(upgrades.undoLastPurchase(player, allies));
    assert(upgrades.getLevel(UpgradeType::Vitality) == 0);
    assert(upgrades.getLevel(UpgradeType::Damage) == 1);
    assert(upgrades.undoLastPurchase(player, allies));
    assert(upgrades.getLevel(UpgradeType::Damage) == 0);
    assert(upgrades.getPendingPurchaseCount() == 0);
    assert(upgrades.getGold() == initialGold);
    assert(!upgrades.undoLastPurchase(player, allies));
}

void testFirstIntermissionConfirmation() {
    IntermissionController controller;
    controller.resetForNewGame();
    controller.sync(true, 1);
    assert(controller.isActive());
    assert(controller.isFirstGuidedIntermission());
    assert(controller.getCompletedWave() == 1);

    assert(controller.handleEnter(0) ==
           IntermissionAction::ShowSkipWarning);
    assert(controller.getPromptState() ==
           IntermissionPromptState::AwaitingSkipConfirmation);
    assert(controller.handleEnter(0) == IntermissionAction::Consumed);
    assert(controller.handleEnterReleased());
    assert(controller.handleEnter(0) == IntermissionAction::BeginNextWave);
    assert(controller.handleEnterReleased());

    controller.sync(false, 2);
    controller.sync(true, 2);
    assert(!controller.isFirstGuidedIntermission());
    assert(controller.handleEnter(0) == IntermissionAction::BeginNextWave);
    controller.handleEnterReleased();

    controller.resetForNewGame();
    controller.sync(true, 1);
    assert(controller.isFirstGuidedIntermission());
}

void testOperationsCancelSkipWarning() {
    IntermissionController controller;
    controller.resetForNewGame();
    controller.sync(true, 1);
    assert(controller.handleEnter(0) ==
           IntermissionAction::ShowSkipWarning);
    controller.handleEnterReleased();
    controller.notifyPurchase(UpgradeType::Damage, false);
    assert(controller.getPromptState() == IntermissionPromptState::Normal);
    assert(controller.getFeedback() == IntermissionFeedback::NotEnoughGold);
    assert(controller.handleEnter(0) ==
           IntermissionAction::ShowSkipWarning);
    controller.handleEnterReleased();
    controller.notifyPurchase(UpgradeType::Damage, true);
    assert(controller.getPromptState() == IntermissionPromptState::Normal);
    assert(controller.getFeedback() == IntermissionFeedback::DamageUpgraded);
    assert(controller.handleEnter(1) == IntermissionAction::BeginNextWave);
    controller.handleEnterReleased();

    controller.sync(false, 2);
    controller.resetForNewGame();
    controller.sync(true, 1);
    assert(controller.handleEnter(0) ==
           IntermissionAction::ShowSkipWarning);
    controller.handleEnterReleased();
    controller.notifyUndo(false);
    assert(controller.getPromptState() == IntermissionPromptState::Normal);
    assert(controller.getFeedback() == IntermissionFeedback::NothingToUndo);
}

void testTutorialModalAndWaveStartsOnce() {
    Map map(15, 15);
    WaveManager waves;
    completeWaveOne(waves, map);
    UpgradeManager upgrades;
    IntermissionController intermission;
    intermission.resetForNewGame();
    intermission.sync(true, waves.getCurrentWave());

    TutorialController tutorial;
    tutorial.resetForNewGame();
    tutorial.handleKeyPressed(TutorialKey::Enter);
    tutorial.handleKeyReleased(TutorialKey::Enter);
    tutorial.finishInputRelease();
    assert(tutorial.handleKeyPressed(TutorialKey::Help) ==
           TutorialTransition::Reopened);
    assert(tutorial.blocksGameplayInput());
    assert(intermission.getPromptState() == IntermissionPromptState::Normal);
    tutorial.handleKeyReleased(TutorialKey::Help);
    assert(tutorial.handleKeyPressed(TutorialKey::Enter) ==
           TutorialTransition::Closed);
    assert(intermission.getPromptState() == IntermissionPromptState::Normal);
    tutorial.handleKeyReleased(TutorialKey::Enter);
    tutorial.finishInputRelease();

    assert(intermission.handleEnter(0) ==
           IntermissionAction::ShowSkipWarning);
    assert(intermission.handleEnter(0) == IntermissionAction::Consumed);
    intermission.handleEnterReleased();
    assert(intermission.handleEnter(0) ==
           IntermissionAction::BeginNextWave);
    waves.startNextWave();
    upgrades.clearUndoHistory();
    assert(waves.getCurrentWave() == 2);
    waves.startNextWave();
    assert(waves.getCurrentWave() == 2);
    assert(upgrades.getPendingPurchaseCount() == 0);
}
}

int main() {
    TextureManager textures;
    testPreviewHasNoSideEffects();
    testUpgradeFormulasAndFailure(textures);
    testUndoLifoAndRetainedCount(textures);
    testFirstIntermissionConfirmation();
    testOperationsCancelSkipWarning();
    testTutorialModalAndWaveStartsOnce();
    return 0;
}
