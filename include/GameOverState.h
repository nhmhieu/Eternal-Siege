#pragma once
#include "State.h"
#include "StateMachine.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include "TextureManager.h"
#include "AudioManager.h"
#include "EndScreenView.h"

class GameOverState : public State {
private:
    StateMachine& stateMachine;
    sf::RenderWindow& window;
    TextureManager& textureManager;
    AudioManager& audioManager;

    EndScreenView view;

public:
    GameOverState(StateMachine& machine, sf::RenderWindow& window,
                  TextureManager& textureManager, AudioManager& audioManager);
    ~GameOverState() override = default;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};
