#include "WinState.h"

#include "MenuState.h"
#include "SetupState.h"

WinState::WinState(StateMachine& machine, sf::RenderWindow& gameWindow,
                   TextureManager& textures, AudioManager& audio)
    : stateMachine(machine),
      window(gameWindow),
      textureManager(textures),
      audioManager(audio),
      view(textures) {}

void WinState::onEnter() {
    audioManager.stopMusic();
    audioManager.playSound("victory");
    view.initialize(EndScreenTheme::Victory);
}

void WinState::onExit() {}

void WinState::handleEvent(const sf::Event& event) {
    const auto restart = [this]() {
        audioManager.playSound("ui_click");
        stateMachine.changeState(std::make_unique<SetupState>(
            stateMachine, window, textureManager, audioManager));
    };
    const auto menu = [this]() {
        audioManager.playSound("ui_click");
        stateMachine.changeState(std::make_unique<MenuState>(
            stateMachine, window, textureManager, audioManager));
    };

    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Enter) {
            restart();
            return;
        }
        if (key->code == sf::Keyboard::Key::Escape) {
            menu();
            return;
        }
    }
    if (const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>();
        mouse && mouse->button == sf::Mouse::Button::Left) {
        const sf::Vector2f position = window.mapPixelToCoords(mouse->position);
        if (view.restartHit(position)) {
            restart();
        } else if (view.menuHit(position)) {
            menu();
        }
    }
}

void WinState::update(float dt) {
    view.update(dt, window);
}

void WinState::render(sf::RenderWindow& target) {
    view.draw(target);
}
