#pragma once
#include "State.h"
#include "StateMachine.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include "TextureManager.h"
#include "AudioManager.h"
#include "EndScreenView.h"

class WinState : public State {
private:
    StateMachine& stateMachine;
    sf::RenderWindow& window;
    TextureManager& textureManager;
    AudioManager& audioManager;

    EndScreenView view;

public:
    WinState(StateMachine& machine, sf::RenderWindow& window,
             TextureManager& textureManager, AudioManager& audioManager);
    ~WinState() override = default;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};
