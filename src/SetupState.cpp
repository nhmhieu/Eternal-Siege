#include "SetupState.h"
#include <iostream>

SetupState::SetupState(StateMachine& machine)
    : stateMachine(machine), map(15, 15)
{
}

void SetupState::onEnter()
{
    if (!font.openFromFile("assets/fonts/arial.ttf"))
    {
        std::cerr << "Failed to load font!" << std::endl;
    }

    startButton.setSize(sf::Vector2f(200.f, 60.f));
    startButton.setFillColor(sf::Color::Green);
    startButton.setPosition(sf::Vector2f(540.f, 600.f));

    startText = std::make_unique<sf::Text>(font);
    startText->setString("BAT DAU");
    startText->setCharacterSize(30);
    startText->setFillColor(sf::Color::White);
    startText->setPosition(sf::Vector2f(595.f, 610.f));

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

            if (canStart &&
                startButton.getGlobalBounds().contains(sf::Vector2f(mousePos)))
            {
                std::cout << "Bat dau game voi "
                          << selectedPositions.size()
                          << " tuong!" << std::endl;

                // stateMachine.changeState(
                //     std::make_unique<GameplayState>(window, selectedPositions));

                return;
            }

            map.handleMouseClick(mousePos.x,
                                 mousePos.y,
                                 selectedPositions,
                                 maxAllies);

            if (static_cast<int>(selectedPositions.size()) == maxAllies)
            {
                canStart = true;
                std::cout << "Da du 4 tuong!" << std::endl;
            }
        }
    }
}

void SetupState::update(float dt)
{
}

void SetupState::render(sf::RenderWindow& window)
{
    map.draw(window);

    if (canStart)
    {
        window.draw(startButton);

        if (startText)
            window.draw(*startText);
    }
}