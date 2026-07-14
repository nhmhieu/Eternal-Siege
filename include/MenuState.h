#pragma once
#include "State.h"
#include "StateMachine.h"
#include <SFML/Graphics.hpp>
#include <memory>

class MenuState : public State {
private:
    StateMachine& stateMachine;
    sf::Font font;
    bool initialized = false;

    // Dùng unique_ptr cho các đối tượng cần Font
    std::unique_ptr<sf::Text> titleText;
    std::unique_ptr<sf::Text> startText;
    std::unique_ptr<sf::Text> exitText;

    // Biến thông thường cho các đối tượng không cần Font
    sf::RectangleShape startButton;
    sf::RectangleShape exitButton;

public:
    MenuState(StateMachine& machine);
    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};