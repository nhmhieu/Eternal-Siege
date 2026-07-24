#include "Game.h"
#include "IntroState.h"
#include "TextureManager.h"
#include "GameplayState.h"

Game::Game()
    : window(sf::VideoMode({ 1280, 720 }), "Eternal Siege") {
    window.setFramerateLimit(60);

    //textureManager.loadTexture("player", "Assets/player.png");
    TextureManager::getInstance().loadTexture("intro", "Assets/Images/intro.png");
    TextureManager::getInstance().loadTexture("menu", "Assets/Images/menu.png");
    TextureManager::getInstance().loadTexture("player", "Assets/Images/player.png");
    TextureManager::getInstance().loadTexture("gameplay","Assets/Images/gameplay.png");


    //stateMachine.changeState(std::make_unique<DummyState>(textureManager));
    stateMachine.changeState(
        std::make_unique<GameplayState>()
    );
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
