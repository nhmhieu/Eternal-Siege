#pragma once

#include "State.h"
#include "StateMachine.h"
#include "TextureManager.h"

#include <SFML/Graphics.hpp>
#include <cstdint>
#include <optional>

class IntroState : public State {
private:
    StateMachine& stateMachine;
    sf::RenderWindow& window;
    TextureManager& textureManager;

    sf::Font font;

    std::optional<sf::Text> line1aText;
    std::optional<sf::Text> line1bText;
    std::optional<sf::Text> createdByText;
    std::optional<sf::Text> groupNameText;

    enum class TextPhase {
        Line1a_FadeIn,
        Line1b_FadeIn,
        Line1_Hold,
        Line1_FadeOut,

        Line2a_FadeIn,
        Line2b_FadeIn,
        Line2_Hold,
        Line2_FadeOut,

        Finished
    };

    TextPhase currentPhase = TextPhase::Line1a_FadeIn;

    float alpha1a = 0.f;
    float alpha1b = 0.f;
    float alpha2a = 0.f;
    float alpha2b = 0.f;
    float holdTimer = 0.f;

    const float fadeSpeed = 150.f;
    const float holdDuration = 1.5f;

    void setupPositions();

public:
    IntroState(
        StateMachine& machine,
        sf::RenderWindow& window,
        TextureManager& textureManager
    );

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};