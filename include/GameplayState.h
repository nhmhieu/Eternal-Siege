#pragma once

#include "State.h"
#include "StateMachine.h"
#include "Player.h"
#include "Monster.h"
#include "CombatManager.h"
#include "GameContext.h"
#include "Ally.h"
#include "WaveManager.h"
#include "UpgradeManager.h"
#include "HUD.h"
#include "TextureManager.h"
#include "Map.h"
#include "AudioManager.h"
#include "Effects.h"
#include "RadiantPulse.h"
#include "BossHealthBar.h"
#include "BossEnrageNotice.h"
#include "TutorialOverlay.h"
#include "IntermissionOverlay.h"
#include "GameplayTransitionGate.h"
#include <memory>
#include <optional>
#include <unordered_set>

class Boss;

class GameplayState : public State {
private:
    StateMachine& stateMachine;
    sf::RenderWindow& window;
    TextureManager& textureManager;
    AudioManager& audioManager;

    // GameplayState la owner duy nhat. GameContext chi muon con tro.
    std::unique_ptr<Player> player;
    std::vector<std::unique_ptr<Ally>> allies;
    std::vector<std::unique_ptr<Monster>> monsters;

    Map map;
    WaveManager waveManager;
    CombatManager combatManager;
    UpgradeManager upgradeManager;
    HUD hud;
    Effects effects;
    RadiantPulse radiantPulse;
    BossHealthBar bossHealthBar;
    BossEnrageNotice bossEnrageNotice;
    TutorialController tutorialController;
    TutorialOverlay tutorialOverlay;
    IntermissionController intermissionController;
    IntermissionOverlay intermissionOverlay;
    std::vector<sf::Vector2i> allyPositions;
    GameContext context;
    bool paused = false;
    GameplayTransitionGate transitionGate;
    sf::Font presentationFont;
    std::optional<sf::Text> bannerText;
    float bannerTimer = 0.f;
    std::unordered_set<const Entity*> deathEffectEntities;
    sf::Vector2f aimWorldPosition{420.f, 360.f};
    float shakeRemaining = 0.f;
    float shakeElapsed = 0.f;
    float shakeStrength = 0.f;
    float lowHealthPulseTime = 0.f;

    void rebuildContext();
    void collectRewardsAndRemoveDead();
    void startNextWave();
    void showWaveBanner(int wave);
    void showBanner(const std::string& text, float duration = 2.5f);
    const Boss* findLivingBoss() const;
    void drawAimingReticle(sf::RenderWindow& target) const;
    void drawLowHealthFeedback(sf::RenderWindow& target) const;
    void drawPauseOverlay(sf::RenderWindow& target) const;
    bool hasHeldGameplayInput() const;
    void finishTutorialInputGuardIfReleased();

public:
    GameplayState(StateMachine& machine, sf::RenderWindow& window,
                  TextureManager& textureManager, AudioManager& audioManager,
                  const Map& setupMap,
                  const std::vector<sf::Vector2i>& allyPositions = {});
    ~GameplayState() override = default;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};
