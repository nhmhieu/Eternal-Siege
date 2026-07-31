#include "GameOverState.h"
#include "MenuState.h"
#include "SetupState.h"
#include <iostream>

GameOverState::GameOverState(StateMachine& machine, sf::RenderWindow& window, TextureManager& textureManager)
    : stateMachine(machine), window(window), textureManager(textureManager)
{
    background.setSize(sf::Vector2f(1280.f, 720.f));
    background.setFillColor(sf::Color(50, 0, 0)); // Dark red background
}

void GameOverState::onEnter() {
    if (!font.openFromFile("assets/fonts/Font.ttf")) {
        std::cerr << "Failed to load font in GameOverState!" << std::endl;
        return;
    }

    titleText = std::make_unique<sf::Text>(font, "GAME OVER", 72);
    titleText->setFillColor(sf::Color::Red);
    sf::FloatRect titleBounds = titleText->getLocalBounds();
    titleText->setOrigin({ titleBounds.position.x + titleBounds.size.x / 2.f,
                           titleBounds.position.y + titleBounds.size.y / 2.f });
    titleText->setPosition({ 640.f, 200.f });

    restartText = std::make_unique<sf::Text>(font, "CHOI LAI", 30);
    restartText->setFillColor(sf::Color::White);
    sf::FloatRect restartBounds = restartText->getLocalBounds();
    restartText->setOrigin({ restartBounds.position.x + restartBounds.size.x / 2.f,
                             restartBounds.position.y + restartBounds.size.y / 2.f });
    restartText->setPosition({ 640.f, 380.f });

    menuText = std::make_unique<sf::Text>(font, "VE MENU", 30);
    menuText->setFillColor(sf::Color::White);
    sf::FloatRect menuBounds = menuText->getLocalBounds();
    menuText->setOrigin({ menuBounds.position.x + menuBounds.size.x / 2.f,
                          menuBounds.position.y + menuBounds.size.y / 2.f });
    menuText->setPosition({ 640.f, 480.f });

    restartButton = std::make_unique<sf::RectangleShape>(sf::Vector2f(220.f, 60.f));
    restartButton->setFillColor(sf::Color(50, 150, 50));
    restartButton->setOrigin({ 110.f, 30.f });
    restartButton->setPosition({ 640.f, 380.f });

    menuButton = std::make_unique<sf::RectangleShape>(sf::Vector2f(220.f, 60.f));
    menuButton->setFillColor(sf::Color(100, 100, 100));
    menuButton->setOrigin({ 110.f, 30.f });
    menuButton->setPosition({ 640.f, 480.f });
}

void GameOverState::onExit() {
}

void GameOverState::handleEvent(const sf::Event& event) {
    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mousePressed->button == sf::Mouse::Button::Left) {
            sf::Vector2f mousePos = sf::Vector2f(mousePressed->position);

            if (restartButton && restartButton->getGlobalBounds().contains(mousePos)) {
                stateMachine.changeState(std::make_unique<SetupState>(stateMachine, window, textureManager));
                return;
            }

            if (menuButton && menuButton->getGlobalBounds().contains(mousePos)) {
                stateMachine.changeState(std::make_unique<MenuState>(stateMachine, window, textureManager));
                return;
            }
        }
    }
}

void GameOverState::update(float) {
    // No update logic needed
}

void GameOverState::render(sf::RenderWindow& window) {
    window.draw(background);
    if (titleText) window.draw(*titleText);
    if (restartButton) window.draw(*restartButton);
    if (restartText) window.draw(*restartText);
    if (menuButton) window.draw(*menuButton);
    if (menuText) window.draw(*menuText);
}
