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
#include "Sword.h"
#include <memory>
#include "Projectiles.h"

class GameplayState : public State {
private:
    StateMachine& stateMachine;
    sf::RenderWindow& window;
    TextureManager& textureManager;

    // GameplayState la owner duy nhat. GameContext chi muon con tro.
    std::unique_ptr<Player> player;
    std::vector<std::unique_ptr<Ally>> allies;
    std::vector<std::unique_ptr<Monster>> monsters;

    Map map;
    WaveManager waveManager;
    CombatManager combatManager;
    UpgradeManager upgradeManager;
    HUD hud;
    std::vector<sf::Vector2i> allyPositions;
    GameContext context;
    bool paused = false;

    void rebuildContext();
    void collectRewardsAndRemoveDead();
    void startNextWave();

public:
    GameplayState(StateMachine& machine, sf::RenderWindow& window,
                  TextureManager& textureManager, const Map& setupMap,
                  const std::vector<sf::Vector2i>& allyPositions = {});
    ~GameplayState() override = default;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};
