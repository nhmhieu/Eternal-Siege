#pragma once

#include "State.h"
#include "Player.h"
#include "Monster.h"
#include "CombatManager.h"
#include "GameContext.h"
#include "Ally.h"
#include "WaveManager.h"
#include "Projectiles.h"

class GameplayState : public State {
private:
    //review code can kiem tra ham huy xem da xoa het mang con tro chua nhe
    Player player;
    // std::vector<Monster*> monsters;
    // std::vector<Ally> allies;
    std :: vector<Projectiles*> projectiles ; 

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