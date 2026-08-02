#pragma once

#include "State.h"
#include "StateMachine.h"
#include "Map.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include "TextureManager.h"
#include "AudioManager.h"

class SetupState : public State
{
private:
    StateMachine& stateMachine;
    sf::RenderWindow& window;
    Map map;
    TextureManager& textureManager;
    AudioManager& audioManager;
    std::vector<sf::Vector2i> selectedPositions;
    int maxAllies = 4;
    bool canStart = false;

    sf::Font font;
    std::unique_ptr<sf::Text> startText;
    std::unique_ptr<sf::Text> titleText;
    std::unique_ptr<sf::Text> instructionText;
    std::unique_ptr<sf::Text> selectedText;
    sf::RectangleShape startButton;

public:
    SetupState(StateMachine& machine, sf::RenderWindow& window,
               TextureManager& textureManager, AudioManager& audioManager);
    ~SetupState() override = default;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};
