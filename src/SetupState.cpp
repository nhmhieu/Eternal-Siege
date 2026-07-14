#include "SetupState.h"

SetupState::SetupState()
    : map(15, 15)
{
}


void SetupState::onEnter()
{
}

void SetupState::onExit()
{
}

#include <iostream>
void SetupState::handleEvent(const sf::Event& event)
{
    if (const auto* mousePress = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mousePress->button == sf::Mouse::Button::Left)
        {
            map.handleMouseClick(mousePress->position.x, mousePress->position.y);
        }
    }
}

void SetupState::update(float dt)
{
}

void SetupState::render(sf::RenderWindow& window)
{
    map.draw(window);
}