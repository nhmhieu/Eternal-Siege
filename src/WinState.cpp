#include "WinState.h"

#include "AssetLocator.h"
#include "KingdomState.h"
#include "LevelSelectState.h"
#include "MenuState.h"
#include "SetupState.h"

#include <algorithm>
#include <string>

namespace {
constexpr sf::Vector2f BUTTON_SIZE{250.f, 50.f};
constexpr sf::Vector2f BUTTON_POSITIONS[] = {
    {375.f, 532.f}, {655.f, 532.f}, {375.f, 592.f}, {655.f, 592.f}
};
constexpr const char* BUTTON_LABELS[] = {
    "CONTINUE", "REPLAY", "KINGDOM", "MAIN MENU"
};
}

WinState::WinState(
    StateMachine& machine, sf::RenderWindow& gameWindow,
    TextureManager& textures, AudioManager& audio,
    GameProgress& gameProgress, RunResult runResult)
    : stateMachine(machine), window(gameWindow), textureManager(textures),
      audioManager(audio), progress(gameProgress), result(runResult)
{
}

void WinState::center(sf::Text& text, sf::Vector2f position) {
    const auto bounds = text.getLocalBounds();
    text.setOrigin({
        bounds.position.x + bounds.size.x / 2.f,
        bounds.position.y + bounds.size.y / 2.f
    });
    text.setPosition(position);
}

void WinState::onEnter() {
    audioManager.stopMusic();
    audioManager.playSound("victory");

    rewardAnimation = 0.f;
    selected = 0;
    transitioning = false;

    const auto path = AssetLocator::find("assets/fonts/Font.ttf");
    if (!path || !resultFont.openFromFile(*path)) {
        return;
    }

    const char* strings[] = {
        "VICTORY", "The Eternal Siege Is Broken",
        "DUNGEON CLEARED", "Dungeon 01 - Ruined Catacombs",
        "4 / 4 Waves Survived", "Final Boss Defeated", "REWARD", "",
        "", "", "Arrows / WASD  Navigate     ENTER  Select     ESC  Main Menu"
    };
    const unsigned sizes[] = {48, 22, 19, 21, 17, 17, 16, 30, 17, 20, 15};
    const sf::Vector2f positions[] = {
        {640, 72}, {640, 116}, {640, 174}, {640, 207},
        {640, 238}, {640, 265}, {470, 324}, {640, 365},
        {640, 405}, {640, 450}, {640, 676}
    };

    for (std::size_t i = 0; i < labels.size(); ++i) {
        labels[i] = std::make_unique<sf::Text>(resultFont, strings[i], sizes[i]);
        labels[i]->setFillColor(
            (i == 0 || i == 2 || i == 6 || i == 7)
                ? sf::Color(246, 204, 95)
                : sf::Color(224, 235, 235)
        );
        center(*labels[i], positions[i]);
    }

    for (std::size_t i = 0; i < buttons.size(); ++i) {
        buttons[i].setSize(BUTTON_SIZE);
        buttons[i].setPosition(BUTTON_POSITIONS[i]);
        buttons[i].setOutlineThickness(2.f);
    }
}

void WinState::onExit() {
}

void WinState::activate(std::size_t index) {
    if (transitioning) {
        return;
    }
    transitioning = true;
    audioManager.playSound("ui_click");

    if (index == 0) {
        stateMachine.changeState(std::make_unique<LevelSelectState>(stateMachine, window, textureManager, audioManager, progress));
        return;
    }
    if (index == 1) {
        stateMachine.changeState(std::make_unique<SetupState>(stateMachine, window, textureManager, audioManager, progress, result.levelId));
        return;
    }
    if (index == 2) {
        stateMachine.changeState(std::make_unique<KingdomState>(stateMachine, window, textureManager, audioManager, progress, true));
        return;
    }
    stateMachine.changeState(std::make_unique<MenuState>(stateMachine, window, textureManager, audioManager, progress));
}

void WinState::handleEvent(const sf::Event& event) {
    if (transitioning) {
        return;
    }

    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Escape) {
            activate(3);
            return;
        }
        if (key->code == sf::Keyboard::Key::Left || key->code == sf::Keyboard::Key::A) {
            selected = (selected % 2 == 0) ? selected + 1 : selected - 1;
        } else if (key->code == sf::Keyboard::Key::Right || key->code == sf::Keyboard::Key::D) {
            selected = (selected % 2 == 0) ? selected + 1 : selected - 1;
        } else if (key->code == sf::Keyboard::Key::Up || key->code == sf::Keyboard::Key::W ||
                   key->code == sf::Keyboard::Key::Down || key->code == sf::Keyboard::Key::S) {
            selected = (selected + 2) % 4;
        } else if (key->code == sf::Keyboard::Key::Enter) {
            activate(selected);
            return;
        }
    }

    if (const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>(); mouse && mouse->button == sf::Mouse::Button::Left) {
        const auto p = window.mapPixelToCoords(mouse->position);
        for (std::size_t i = 0; i < buttons.size(); ++i) {
            if (buttons[i].getGlobalBounds().contains(p)) {
                selected = i;
                activate(i);
                return;
            }
        }
    }
}

void WinState::update(float dt) {
    rewardAnimation = std::min(1.f, rewardAnimation + dt / .75f);
    if (labels[7]) {
        labels[7]->setString("+" + std::to_string(static_cast<int>(result.goldEarned * rewardAnimation)) + " GOLD");
        center(*labels[7], {640, 365});
    }
    if (labels[8]) {
        labels[8]->setString("Total Gold: " + std::to_string(result.totalGold));
        center(*labels[8], {640, 405});
    }

    const auto mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    for (std::size_t i = 0; i < buttons.size(); ++i) {
        if (buttons[i].getGlobalBounds().contains(mouse)) {
            selected = i;
        }
    }
}

void WinState::render(sf::RenderWindow& target) {
    target.setView(target.getDefaultView());
    target.clear({5, 13, 22});

    sf::RectangleShape veil({1280, 720});
    veil.setFillColor({3, 12, 19, 245});
    target.draw(veil);

    sf::RectangleShape panel({720, 610});
    panel.setPosition({280, 36});
    panel.setFillColor({10, 25, 35, 248});
    panel.setOutlineColor({220, 174, 76, 220});
    panel.setOutlineThickness(2.f);
    target.draw(panel);

    sf::RectangleShape reward({430, 142});
    reward.setPosition({425, 296});
    reward.setFillColor({14, 38, 45, 245});
    reward.setOutlineColor({74, 224, 190, 190});
    reward.setOutlineThickness(1.5f);
    target.draw(reward);

    for (std::size_t i = 0; i < buttons.size(); ++i) {
        buttons[i].setFillColor(i == selected ? sf::Color(45, 87, 84) : sf::Color(19, 40, 50));
        buttons[i].setOutlineColor(i == selected ? sf::Color(250, 205, 98) : sf::Color(76, 145, 143));
        target.draw(buttons[i]);

        if (labels[9]) {
            sf::Text label(resultFont, BUTTON_LABELS[i], 19);
            label.setFillColor({235, 241, 236});
            center(label, BUTTON_POSITIONS[i] + BUTTON_SIZE / 2.f);
            target.draw(label);
        }
    }

    for (const auto& label : labels) {
        if (label) {
            target.draw(*label);
        }
    }
}
