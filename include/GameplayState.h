#pragma once

#include "State.h"
#include "Player.h"
#include "Monster.h"
#include "CombatManager.h"
#include "GameContext.h"
#include "Ally.h"
#include "WaveManager.h"

class GameplayState : public State {
private:
    Player player;
    std::vector<Monster*> monsters;
    std::vector<Ally> allies;
    WaveManager waveManager;
    GameContext context;
    sf::RenderWindow& window;
    CombatManager combatManager;

public:
    GameplayState(sf::RenderWindow& window, const std::vector<sf::Vector2i>& allyPositions ={});
    ~GameplayState() override;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};