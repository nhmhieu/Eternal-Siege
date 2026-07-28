#pragma once
#include "State.h"
#include "StateMachine.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include "TextureManager.h"

class MenuState : public State {
private:
    StateMachine& stateMachine;
    sf::RenderWindow& window;
    sf::Font font;
    //TextureManager& textureManager;
    std::unique_ptr<sf::Sprite> background;

    // Dùng unique_ptr cho các đối tượng cần Font
    std::unique_ptr<sf::Text> titleText;
    std::unique_ptr<sf::Text> startText;
    std::unique_ptr<sf::Text> exitText;

    // Biến thông thường cho các đối tượng không cần Font
    std::unique_ptr<sf::RectangleShape> startButton;
    std::unique_ptr<sf::RectangleShape> exitButton;

public:
    MenuState(StateMachine& machine, sf::RenderWindow& window, TextureManager& textureManager);
    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};