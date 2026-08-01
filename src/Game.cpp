#include "Game.h"
#include "IntroState.h"
#include "TextureManager.h"
#include "GameplayState.h"
#include "TextureManager.h"
#include "GameplayState.h"
#include "GameContext.h"
#include "IntroState.h"


Game::Game()
    : window(sf::VideoMode({ 1280, 720 }), "Eternal Siege") {
    window.setFramerateLimit(60);

    //textureManager.loadTexture("player", "Assets/player.png");
    TextureManager::getInstance().loadTexture("intro", "assets/images/intro.png");
    TextureManager::getInstance().loadTexture("menu", "assets/images/menu.png");
    TextureManager::getInstance().loadTexture("player", "assets/images/tower2.png");
    TextureManager::getInstance().loadTexture("gameplay", "assets/images/gameplay.png");
    TextureManager::getInstance().loadTexture("gameover", "assets/images/gameover.png");
    TextureManager::getInstance().loadTexture("setup", "assets/images/setup.png");
    TextureManager::getInstance().loadTexture("win", "assets/images/win.png");
    TextureManager::getInstance().loadTexture("title", "assets/images/title.png");
    TextureManager::getInstance().loadTexture("monster", "assets/images/monster.png");



    //stateMachine.changeState(std::make_unique<DummyState>(textureManager));
    stateMachine.changeState(
        std::make_unique<GameplayState>(stateMachine, window)
    );
}
Game::~Game() {
    // Dọn dẹp tài nguyên nếu cần, hoặc để rỗng
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
