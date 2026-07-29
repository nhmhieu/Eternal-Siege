#pragma once
#include "State.h"
#include "StateMachine.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include "TextureManager.h"

class GameOverState : public State {
private:
    StateMachine& stateMachine;
    sf::RenderWindow& window;

    //sf::RectangleShape background;
    std::unique_ptr<sf::Sprite> background;

    sf::Font font;

    std::unique_ptr<sf::Text> titleText;
    std::unique_ptr<sf::Text> restartText;
    std::unique_ptr<sf::Text> menuText;
    std::unique_ptr<sf::RectangleShape> restartButton;
    std::unique_ptr<sf::RectangleShape> menuButton;

public:
    GameOverState(StateMachine& machine, sf::RenderWindow& window);
    ~GameOverState() override = default;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};