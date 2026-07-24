#pragma once
#include "State.h"
#include "Player.h"
#include "TextureManager.h"
#include <memory>

class GameplayState : public State {
private:
    std::unique_ptr<sf::Sprite> background;
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