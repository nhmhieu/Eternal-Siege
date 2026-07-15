#pragma once
#include "State.h"
#include "StateMachine.h"
#include <SFML/Graphics.hpp>
#include <memory>

class LoseState : public State {
private:
    StateMachine& stateMachine;
    sf::Font font;
    bool initialized = false;

    std::unique_ptr<sf::Text> loseText;
    std::unique_ptr<sf::Text> restartText;
    std::unique_ptr<sf::Text> exitText;

    sf::RectangleShape restartButton;
    sf::RectangleShape exitButton;

public:
    LoseState(StateMachine& machine);
    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};