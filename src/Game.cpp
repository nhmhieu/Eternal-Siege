#include "Game.h"
#include "DummyState.h"
#include "GameplayState.h"
#include "GameContext.h"


Game::Game()
    : window(sf::VideoMode({ 1280, 720 }), "Eternal Siege") {
    window.setFramerateLimit(60);
    stateMachine.changeState(std::make_unique<MenuState>(stateMachine, window));
}


Game :: ~Game(){
    
}

void Game::run() {

    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
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