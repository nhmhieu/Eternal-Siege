#include "Game.h"
#include "TextureManager.h"
#include "GameplayState.h"
#include "GameContext.h"
#include "IntroState.h"
#include "AssetLocator.h"
#include "Constants.h"
#include <optional>
#include <algorithm>


Game::Game()
    : window(sf::VideoMode({ 1280, 720 }), "Eternal Siege") {
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(GameConfig::KEY_REPEAT_ENABLED);
    if (const auto iconPath =
            AssetLocator::find("assets/images/ui/window_icon.png")) {
        sf::Image icon;
        if (icon.loadFromFile(*iconPath)) {
            window.setIcon(icon.getSize(), icon.getPixelsPtr());
        }
    }
    stateMachine.changeState(std::make_unique<IntroState>(
        stateMachine, window, textureManager, audioManager, progress));
}


Game :: ~Game(){
    
}

void Game::run() {

    stateMachine.consumeTransitionApplied();
    clock.restart();

    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                break;
            }
            if (const auto* key = event->getIf<sf::Event::KeyPressed>();
                key && key->code == sf::Keyboard::Key::M) {
                audioManager.handleMuteKeyPressed();
                continue;
            }
            if (const auto* key = event->getIf<sf::Event::KeyReleased>();
                key && key->code == sf::Keyboard::Key::M) {
                audioManager.handleMuteKeyReleased();
                continue;
            }
            stateMachine.handleEvent(*event);
            if (stateMachine.consumeTransitionApplied()) clock.restart();
        }

        if (!window.isOpen()) {
            break;
        }

        // Avoid a huge simulation jump after dragging/debug-pausing the window.
        const float dt = std::min(clock.restart().asSeconds(), 0.1f);

        audioManager.update(dt);
        stateMachine.update(dt);
        if (stateMachine.consumeTransitionApplied()) clock.restart();
        window.clear();
        stateMachine.render(window);
        window.display();
    }
}
