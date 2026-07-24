#pragma once
#include "State.h"
#include "Player.h"

class GameplayState : public State {
private:
    Player player;
     std::vector<sf::Vector2i> allyPositions;

public:
    GameplayState(const std::vector<sf::Vector2i>& positions);
    ~GameplayState() override = default;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};