#include "Game.h"
#include "TextureManager.h"
#include "GameplayState.h"
#include "GameContext.h"
#include "IntroState.h"
#include <optional>
#include <algorithm>


Game::Game()
    : window(sf::VideoMode({ 1280, 720 }), "Eternal Siege") {
    window.setFramerateLimit(60);
    stateMachine.changeState(std::make_unique<IntroState>(stateMachine, window, textureManager));
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
        // Avoid a huge simulation jump after dragging/debug-pausing the window.
        const float dt = std::min(clock.restart().asSeconds(), 0.1f);

        stateMachine.update(dt);
        window.clear();
        stateMachine.render(window);
        window.display();
    }
}
