#include "GameOverState.h"

#include "MenuState.h"
#include "SetupState.h"
#include "KingdomState.h"
#include "LevelSelectState.h"

GameOverState::GameOverState(StateMachine& machine,
                             sf::RenderWindow& gameWindow,
                             TextureManager& textures,
                             AudioManager& audio, GameProgress& gameProgress,
                             LevelId levelId)
    : stateMachine(machine),
      window(gameWindow),
      textureManager(textures),
      audioManager(audio),
      progress(gameProgress), selectedLevelId(levelId),
      view(textures) {}

void GameOverState::onEnter() {
    audioManager.stopMusic();
    audioManager.playSound("defeat");
    view.initialize(EndScreenTheme::Defeat);
}

void GameOverState::onExit() {}

void GameOverState::handleEvent(const sf::Event& event) {
    const auto restart = [this]() {
        audioManager.playSound("ui_click");
        stateMachine.changeState(std::make_unique<SetupState>(
            stateMachine, window, textureManager, audioManager, progress,
            selectedLevelId));
    };
    const auto menu = [this]() {
        audioManager.playSound("ui_click");
        stateMachine.changeState(std::make_unique<MenuState>(
            stateMachine, window, textureManager, audioManager, progress));
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
        if (key->code == sf::Keyboard::Key::K) {
            stateMachine.changeState(std::make_unique<KingdomState>(
                stateMachine, window, textureManager, audioManager, progress, true));
            return;
        }
        if (key->code == sf::Keyboard::Key::L) {
            stateMachine.changeState(std::make_unique<LevelSelectState>(
                stateMachine, window, textureManager, audioManager, progress));
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

void GameOverState::update(float dt) {
    view.update(dt, window);
}

void GameOverState::render(sf::RenderWindow& target) {
    view.draw(target);
}
