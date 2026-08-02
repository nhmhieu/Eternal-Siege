#include "SetupState.h"
#include "GameplayState.h"
#include "TextureManager.h"
#include "AssetLocator.h"
#include <iostream>

SetupState::SetupState(StateMachine& machine, sf::RenderWindow& window,
                       TextureManager& textureManager, AudioManager& audioManager)
    : stateMachine(machine), window(window), map(15, 15),
      textureManager(textureManager), audioManager(audioManager)
{
    map.setTextureManager(textureManager);
}

void SetupState::onEnter()
{
    // 1. Load font
    const auto fontPath = AssetLocator::find("assets/fonts/Font.ttf");
    bool fontLoaded = fontPath && font.openFromFile(*fontPath);
    if (!fontLoaded) {
        std::cerr << "Failed to load font!" << std::endl;
        // Không tạo text nếu font lỗi
        startText.reset();
    }
    else {
        // 2. Tạo text chỉ khi font thành công
        startText = std::make_unique<sf::Text>(font, "START SIEGE", 22);
        startText->setFillColor(sf::Color::White);
        startText->setPosition(sf::Vector2f(1065.f, 622.f));
        titleText = std::make_unique<sf::Text>(font, "DEPLOY THE GUARDIANS", 28);
        titleText->setFillColor(sf::Color(236, 202, 127));
        titleText->setPosition({760.f, 48.f});
        instructionText = std::make_unique<sf::Text>(
            font, "Select exactly four walkable tiles.\nClick a selected tile to undo.", 15);
        instructionText->setFillColor(sf::Color(164, 183, 191));
        instructionText->setLineSpacing(1.35f);
        instructionText->setPosition({760.f, 102.f});
        selectedText = std::make_unique<sf::Text>(font, "Selected 0/4", 20);
        selectedText->setPosition({760.f, 198.f});
    }

    // 3. Các thành phần khác (không cần font)
    startButton.setSize(sf::Vector2f(200.f, 58.f));
    startButton.setFillColor(sf::Color(31, 42, 49));
    startButton.setOutlineColor(sf::Color(102, 104, 100));
    startButton.setOutlineThickness(2.f);
    startButton.setPosition(sf::Vector2f(1040.f, 595.f));

    canStart = false;
    selectedPositions.clear();
}

void SetupState::onExit()
{
}

void SetupState::handleEvent(const sf::Event& event)
{
    if (const auto* mousePress = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mousePress->button == sf::Mouse::Button::Left)
        {
            sf::Vector2i mousePos = mousePress->position;

            if (canStart && startButton.getGlobalBounds().contains(sf::Vector2f(mousePos)))
            {
                audioManager.playSound("ui_click");
                stateMachine.changeState(std::make_unique<GameplayState>(
                    stateMachine, window, textureManager, audioManager,
                    map, selectedPositions));
                return;
            }

            const std::vector<sf::Vector2i> selectionBefore =
                selectedPositions;
            map.handleMouseClick(
                mousePos.x, mousePos.y, selectedPositions, maxAllies);
            if (selectedPositions != selectionBefore) {
                audioManager.playSound("ui_click");
            }
            canStart = static_cast<int>(selectedPositions.size()) == maxAllies;
            if (selectedText) {
                selectedText->setString("Selected " +
                    std::to_string(selectedPositions.size()) + "/4");
                selectedText->setFillColor(canStart
                    ? sf::Color(100, 235, 190) : sf::Color(185, 198, 204));
            }
            startButton.setFillColor(canStart
                ? sf::Color(36, 104, 91) : sf::Color(31, 42, 49));
            startButton.setOutlineColor(canStart
                ? sf::Color(235, 191, 96) : sf::Color(102, 104, 100));
        }
    }
}

void SetupState::update(float)
{}

void SetupState::render(sf::RenderWindow& window)
{
    map.draw(window);
    sf::RectangleShape sidePanel({560.f, 720.f});
    sidePanel.setPosition({720.f, 0.f});
    sidePanel.setFillColor(sf::Color(8, 18, 28, 240));
    sidePanel.setOutlineColor(sf::Color(195, 147, 72, 170));
    sidePanel.setOutlineThickness(2.f);
    window.draw(sidePanel);

    const sf::Vector2f mouseWorld = window.mapPixelToCoords(
        sf::Mouse::getPosition(window));
    const sf::Vector2i hoverCell = map.worldToGrid(mouseWorld);
    if (map.isInside(hoverCell) && map.isWalkable(hoverCell)) {
        sf::RectangleShape hover({46.f, 46.f});
        hover.setPosition({hoverCell.x * 48.f + 1.f,
                           hoverCell.y * 48.f + 1.f});
        hover.setFillColor(sf::Color(80, 220, 205, 35));
        hover.setOutlineColor(sf::Color(98, 235, 213));
        hover.setOutlineThickness(2.f);
        window.draw(hover);
    }
    for (const sf::Vector2i cell : selectedPositions) {
        sf::RectangleShape selected({42.f, 42.f});
        selected.setPosition({cell.x * 48.f + 3.f, cell.y * 48.f + 3.f});
        selected.setFillColor(sf::Color(70, 205, 195, 50));
        selected.setOutlineColor(sf::Color(255, 209, 105));
        selected.setOutlineThickness(3.f);
        window.draw(selected);
    }

    window.draw(startButton);
    if (titleText) window.draw(*titleText);
    if (instructionText) window.draw(*instructionText);
    if (selectedText) window.draw(*selectedText);
    if (startText) {
        startText->setFillColor(canStart ? sf::Color::White
                                         : sf::Color(112, 121, 126));
        window.draw(*startText);
    }
}
