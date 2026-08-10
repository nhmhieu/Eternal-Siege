#pragma once
#include "State.h"
#include "StateMachine.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include "TextureManager.h"
#include "AudioManager.h"
#include "GameProgress.h"
#include <array>
#include <memory>

class WinState : public State {
private:
    StateMachine& stateMachine;
    sf::RenderWindow& window;
    TextureManager& textureManager;
    AudioManager& audioManager;
    GameProgress& progress;
    RunResult result;
    sf::Font resultFont;
    float rewardAnimation = 0.f;
    std::size_t selected = 0;
    bool transitioning = false;
    std::array<sf::RectangleShape, 4> buttons;
    std::array<std::unique_ptr<sf::Text>, 11> labels;

    void activate(std::size_t index);
    static void center(sf::Text& text, sf::Vector2f position);

public:
    WinState(StateMachine& machine, sf::RenderWindow& window,
             TextureManager& textureManager, AudioManager& audioManager,
             GameProgress& progress, RunResult result);
    ~WinState() override = default;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};
