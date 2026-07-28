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

    // Entity management (dùng raw pointer trong context)
    std::unique_ptr<Player> player;
    // Không cần vector riêng nữa, dùng context
    Map map;
    WaveManager waveManager;
    CombatManager combatManager;
    std::unique_ptr<Sword> sword;

    // Context (chứa tất cả entity)
    GameContext context;

public:
    GameplayState(StateMachine& machine, sf::RenderWindow& window, TextureManager& textureManager, const std::vector<sf::Vector2i>& allyPositions = {});
    ~GameplayState() override;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};