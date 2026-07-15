#pragma once

#include "State.h"
#include "Player.h"
#include "Monster.h"
#include "CombatManager.h"
#include "GameContext.h"

class GameplayState : public State {
private:
    Player player;
    std::vector<Monster*> monsters;
    GameContext context;
    sf::RenderWindow& window;
    CombatManager combatManager;

public:
    GameplayState(sf::RenderWindow& window);
    ~GameplayState() override;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};