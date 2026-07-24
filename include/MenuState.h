#pragma once
#include "State.h"
#include "StateMachine.h"
#include <SFML/Graphics.hpp>
#include <memory>   // Thêm dòng này

class MenuState : public State {
private:
    StateMachine& stateMachine;
    sf::Font font;
    bool initialized = false;

    std::unique_ptr<sf::Sprite> background;

    sf::Text titleText;
    sf::RectangleShape startButton;
    sf::Text startText;
    sf::RectangleShape exitButton;
    sf::Text exitText;  

public:
    MenuState(StateMachine& machine);
    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};