#pragma once

#include "AudioManager.h"
#include "State.h"
#include "StateMachine.h"
#include "TextureManager.h"

#include <SFML/Graphics.hpp>
#include <array>
#include <optional>

class IntroState : public State {
public:
    static constexpr std::array<float, 4> FRAME_DURATIONS{{
        3.5f, 4.5f, 5.f, 5.f
    }};

    IntroState(StateMachine& machine, sf::RenderWindow& window,
               TextureManager& textureManager, AudioManager& audioManager);
    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;

private:
    void showFrame();
    void finishIntro();

    StateMachine& stateMachine;
    sf::RenderWindow& window;
    TextureManager& textureManager;
    AudioManager& audioManager;
    sf::Font font;
    std::optional<sf::Text> storyText;
    std::optional<sf::Text> hintText;
    std::optional<sf::Sprite> logoSprite;
    int frameIndex = 0;
    float frameTimer = 0.f;
    bool transitionRequested = false;
    bool advanceKeyHeld = false;
};
