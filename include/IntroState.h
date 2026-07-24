#pragma once
#include "State.h"
#include <SFML/Graphics.hpp>
#include <memory>
class StateMachine;

class IntroState : public State {
private:
    StateMachine& machine;
    float displayTime;
    bool isDone;
    //sf::RectangleShape background;
    std::unique_ptr<sf::Sprite> background;
    sf::Font font;
    std::unique_ptr<sf::Text> gameTitle;


public:
    IntroState(StateMachine& machine);

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};