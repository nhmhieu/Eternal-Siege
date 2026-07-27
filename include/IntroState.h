#pragma once
#include "State.h"
#include "StateMachine.h"
#include <SFML/Graphics.hpp>
#include <cstdint>
#include <optional>

class IntroState : public State {
private:
    StateMachine& stateMachine;
    sf::RenderWindow& window;
    sf::Font font;

    // Dòng 1a & 1b: "from nowhere" & "of the universe"
    std::optional<sf::Text> line1aText;
    std::optional<sf::Text> line1bText;

    // Dòng 2a & 2b: "created by" & "Ngo0Group"
    std::optional<sf::Text> createdByText;
    std::optional<sf::Text> groupNameText;

    enum class TextPhase {
        Line1a_FadeIn,   // "from nowhere" hiện dần
        Line1b_FadeIn,   // "of the universe" hiện dần ở dưới
        Line1_Hold,      // Giữ cả 2 dòng 1a & 1b sáng
        Line1_FadeOut,   // Cả 2 dòng 1a & 1b cùng mờ đi

        Line2a_FadeIn,   // "created by" hiện dần
        Line2b_FadeIn,   // "Ngo0Group" hiện dần ở dưới
        Line2_Hold,      // Giữ cả 2 dòng 2a & 2b sáng
        Line2_FadeOut,   // Cả 2 dòng 2a & 2b cùng mờ đi

        Finished
    };
    TextPhase currentPhase = TextPhase::Line1a_FadeIn;

    // Alpha riêng cho từng câu chữ
    float alpha1a = 0.f;
    float alpha1b = 0.f;
    float alpha2a = 0.f; // Alpha cho "created by"
    float alpha2b = 0.f; // Alpha cho "Ngo0Group"

    float holdTimer = 0.f;

    const float fadeSpeed = 150.f;
    const float holdDuration = 1.5f;

    void setupPositions();

public:
    IntroState(StateMachine& machine, sf::RenderWindow& window);
    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};