#include "SetupState.h"
#include "GameplayState.h"
#include "TextureManager.h"
#include <iostream>

SetupState::SetupState(StateMachine& machine, sf::RenderWindow& window, TextureManager& textureManager)
    : stateMachine(machine), window(window), textureManager(textureManager), map(15, 15)
{
}

void SetupState::onEnter()
{
    // 1. Load font
    bool fontLoaded = font.openFromFile("assets/fonts/Font.ttf");
    if (!fontLoaded) {
        std::cerr << "Failed to load font!" << std::endl;
        // Không tạo text nếu font lỗi
        startText.reset();
    }
    else {
        // 2. Tạo text chỉ khi font thành công
        startText = std::make_unique<sf::Text>(font, "BAT DAU", 30);
        startText->setFillColor(sf::Color::White);
        startText->setPosition(sf::Vector2f(595.f, 610.f));
    }

    // 3. Các thành phần khác (không cần font)
    startButton.setSize(sf::Vector2f(200.f, 60.f));
    startButton.setFillColor(sf::Color::Green);
    startButton.setPosition(sf::Vector2f(540.f, 600.f));

    canStart = false;
    selectedPositions.clear();
}

void SetupState::onExit()
{
    std::cout << "Thoat SetupState!" << std::endl;
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
                std::cout << "Bat dau game voi " << selectedPositions.size() << " tuong!" << std::endl;
                stateMachine.changeState(std::make_unique<GameplayState>(stateMachine, window, textureManager, selectedPositions));
                return;
            }

            map.handleMouseClick(mousePos.x, mousePos.y, selectedPositions, maxAllies);

            if (static_cast<int>(selectedPositions.size()) == maxAllies)
            {
                canStart = true;
                std::cout << "Da du 4 tuong!" << std::endl;
            }
        }
    }
}

void SetupState::update(float dt)
{}

void SetupState::render(sf::RenderWindow& window)
{
    map.draw(window);
    if (canStart)
    {
        window.draw(startButton);
        if (startText) window.draw(*startText);
    }
}