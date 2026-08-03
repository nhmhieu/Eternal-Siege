#pragma once

#include "AllyPlacementModel.h"
#include "AudioManager.h"
#include "Map.h"
#include "State.h"
#include "StateMachine.h"
#include "TextureManager.h"

#include <SFML/Graphics.hpp>

#include <array>
#include <memory>
#include <optional>

class SetupState : public State {
private:
    struct HeroCardUi {
        sf::RectangleShape panel;
        sf::RectangleShape portrait;
        std::unique_ptr<sf::Text> name;
        std::unique_ptr<sf::Text> details;
        std::unique_ptr<sf::Text> status;
    };

    StateMachine& stateMachine;
    sf::RenderWindow& window;
    Map map;
    TextureManager& textureManager;
    AudioManager& audioManager;
    AllyPlacementModel placements;
    std::optional<AllyType> selectedAlly;

    sf::Font font;
    bool fontLoaded = false;
    bool inputArmed = false;
    bool transitionRequested = false;
    float feedbackRemaining = 0.f;

    sf::RectangleShape sidePanel;
    sf::RectangleShape startButton;
    std::array<HeroCardUi, AllyPlacementModel::ALLY_COUNT> heroCards;
    std::array<const sf::Texture*, AllyPlacementModel::ALLY_COUNT>
        heroTextures{};
    std::array<sf::IntRect, AllyPlacementModel::ALLY_COUNT>
        heroVisibleBounds{};

    std::unique_ptr<sf::Text> titleText;
    std::unique_ptr<sf::Text> instructionText;
    std::unique_ptr<sf::Text> deployedText;
    std::unique_ptr<sf::Text> controlsText;
    std::unique_ptr<sf::Text> feedbackText;
    std::unique_ptr<sf::Text> startText;
    std::unique_ptr<sf::Text> startHintText;

    void loadResources();
    void createTextUi();
    void updateLayout();
    void refreshUi();
    bool isDeployableCell(sf::Vector2i cell) const;
    std::optional<sf::Vector2i> mouseCell(sf::Vector2i pixel) const;
    void selectAlly(AllyType type);
    void deploySelected(sf::Vector2i cell);
    void removeAt(sf::Vector2i cell);
    void resetFormation();
    void setFeedback(const char* message, sf::Color color);
    void startBattle();

    void drawPlacementHints(sf::RenderWindow& target) const;
    void drawPlacedHeroes(sf::RenderWindow& target) const;
    void drawPanel(sf::RenderWindow& target);

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
