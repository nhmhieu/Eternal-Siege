#pragma once

#include "AudioManager.h"
#include "State.h"
#include "StateMachine.h"
#include "TextureManager.h"
#include "GameProgress.h"

#include <SFML/Graphics.hpp>
#include <array>
#include <optional>
#include <string>

class IntroState : public State {
public:
    static constexpr std::array<float, 5> FRAME_DURATIONS{{
        3.5f, 6.0f, 6.0f, 6.0f, 7.0f
    }};

    IntroState(StateMachine& machine, sf::RenderWindow& window,
               TextureManager& textureManager, AudioManager& audioManager,
               GameProgress& progress);
    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;

private:
    void showFrame(int targetFrame);
    void finishIntro();
    std::string wrapText(const sf::Font& font, const std::string& str, unsigned int characterSize, float maxWidth);

    StateMachine& stateMachine;
    sf::RenderWindow& window;
    TextureManager& textureManager;
    AudioManager& audioManager;
    GameProgress& progress;

    sf::Font font;
    std::optional<sf::Text> headingText;
    std::optional<sf::Text> subtitleText;
    std::optional<sf::Text> bodyText;
    std::optional<sf::Text> emphasisText;
    std::optional<sf::Text> hintText;
    std::optional<sf::Text> createdByText;
    std::optional<sf::Text> groupNameText;
    std::optional<sf::Sprite> logoSprite;

    int frameIndex = 0;
    float frameTimer = 0.f;
    float fadeAlpha = 1.f;
    int pendingFrameIndex = -1;
    bool transitionRequested = false;
    bool advanceKeyHeld = false;
};
