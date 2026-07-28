#pragma once
#include "State.h"
#include "StateMachine.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include "TextureManager.h"

class IntroState : public State {
private:
    StateMachine& machine;
    sf::RenderWindow& window;
    TextureManager& textureManager;

    float displayTime;
    bool isDone;
    //sf::RectangleShape background;
    std::unique_ptr<sf::Sprite> background;
    sf::Font font;
    std::unique_ptr<sf::Text> gameTitle;

public:
    IntroState(StateMachine& machine, sf::RenderWindow& window, TextureManager& textureManager);
    ~IntroState() override = default;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};