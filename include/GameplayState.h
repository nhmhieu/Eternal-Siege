#pragma once

#include "State.h"
#include "Player.h"
#include "Monster.h"
#include "CombatManager.h"
#include "GameContext.h"
#include "Ally.h"
#include "WaveManager.h"
#include "TextureManager.h"

class GameplayState : public State {
private:
    TextureManager& textureManager;
    std::unique_ptr<Player> player;
    std::vector<Monster*> monsters;
    std::vector<std::unique_ptr<Ally>> allies;
    WaveManager waveManager;
    GameContext context;
    sf::RenderWindow& window;
    CombatManager combatManager;

public:
    GameplayState(sf::RenderWindow& window, TextureManager& textureManager, const std::vector<sf::Vector2i>& allyPositions ={});
    ~GameplayState() override;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};