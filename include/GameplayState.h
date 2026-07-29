#pragma once

#include "State.h"
#include "StateMachine.h"
#include "Player.h"
#include "Monster.h"
#include "CombatManager.h"
#include "GameContext.h"
#include "Ally.h"
#include "WaveManager.h"
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

    // Entity management (dng raw pointer trong context)
    std::unique_ptr<Player> player;
    // Khng c?n vector ring n?a, dng context
    Map map;
    WaveManager waveManager;
    CombatManager combatManager;
    bool paused = false;

    // Context (ch?a t?t c? entity)
    GameContext context;

public:
    GameplayState(StateMachine& machine, sf::RenderWindow& window, TextureManager& textureManager,const Map& setupMap, const std::vector<sf::Vector2i>& allyPositions = {});
    ~GameplayState() override;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};
