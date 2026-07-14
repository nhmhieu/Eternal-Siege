#pragma once

#include "State.h"
#include "StateMachine.h"
#include "Map.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

class SetupState : public State
{
private:
    StateMachine& stateMachine;
    Map map;
    std::vector<sf::Vector2i> selectedPositions;
    int maxAllies = 4;
    bool canStart = false;

    sf::Font font;
    std::unique_ptr<sf::Text> startText;
    sf::RectangleShape startButton;

public:
    SetupState(StateMachine& machine);
    ~SetupState() override = default;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};