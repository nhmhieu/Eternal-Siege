#pragma once

#include "State.h"
#include "Map.h"
#include "Player.h"
#include "Ally.h"
#include <memory>

#include <vector>

class GameplayState : public State
{
private:
    Map map;
    Player player;

    std::vector<sf::Vector2i> allyPositions;
    std::vector<std::unique_ptr<Ally>> allies;

public:
    GameplayState(
        const Map& setupMap,
        const std::vector<sf::Vector2i>& positions);

    ~GameplayState() override = default;

    void onEnter() override;
    void onExit() override;

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};

