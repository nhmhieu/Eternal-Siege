#include "Game.h"
#include "SetupState.h"
#include <iostream>

Game::Game()
    : window(sf::VideoMode({1280, 720}), "Eternal Siege")
{
    window.setFramerateLimit(60);

    stateMachine.changeState(
    std::make_unique<SetupState>(stateMachine)
);
}

void Game::run()
{
    while (window.isOpen())
    {
        while (const std::optional<sf::Event> event = window.pollEvent())
        {
            std::cout << "Game got an event" << std::endl;
            if (event->is<sf::Event::Closed>())
                window.close();

            stateMachine.handleEvent(*event);
        }

        float dt = clock.restart().asSeconds();

        stateMachine.update(dt);

        window.clear();

        stateMachine.render(window);

        window.display();
    }
}