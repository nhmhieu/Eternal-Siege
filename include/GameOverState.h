#pragma once
#include "State.h"
#include "StateMachine.h"
#include <SFML/Graphics.hpp>
#include <memory>

class GameOverState : public State {
private:
    StateMachine& stateMachine;
    sf::Font font;
    bool initialized = false;

    std::unique_ptr<sf::Text> gameOverText;
    std::unique_ptr<sf::Text> retryText;
    std::unique_ptr<sf::Text> exitText;

    sf::RectangleShape retryButton;
    sf::RectangleShape exitButton;

public:
    GameOverState(StateMachine& machine);
    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};
