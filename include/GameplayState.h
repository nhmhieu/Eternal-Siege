#pragma once
#include "State.h"
#include "Player.h"

class GameplayState : public State {
private:
    Player player;

public:
    GameplayState() = default;
    ~GameplayState() override = default;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};