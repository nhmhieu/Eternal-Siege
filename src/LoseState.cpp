#include "LoseState.h"
#include "MenuState.h"
#include <iostream>

LoseState::LoseState(StateMachine& machine)
    : stateMachine(machine), initialized(false) {
}

void LoseState::onEnter() {
    std::cout << "--- DANG MO LOSE STATE ---" << std::endl;

    if (!font.openFromFile("assets/Montserrat-Italic.ttf")) {
        std::cerr << "LoseState: Failed to load font!" << std::endl;
        initialized = false;
        return;
    }
    initialized = true;

    // 1. Chữ Thất Bại (Màu đỏ u ám)
    loseText = std::make_unique<sf::Text>(font, "GAME OVER", 80);
    loseText->setFillColor(sf::Color::Red);
    loseText->setPosition({ 400.f, 150.f });

    // 2. Nút Thử Lại
    restartButton.setSize(sf::Vector2f(220.f, 60.f));
    restartButton.setFillColor(sf::Color(70, 70, 70)); // Màu xám tối
    restartButton.setPosition({ 530.f, 350.f });

    restartText = std::make_unique<sf::Text>(font, "TRY AGAIN", 25);
    restartText->setFillColor(sf::Color::White);
    restartText->setPosition({ 575.f, 365.f });

    // 3. Nút Thoát về Menu
    exitButton.setSize(sf::Vector2f(220.f, 60.f));
    exitButton.setFillColor(sf::Color(150, 50, 50));
    exitButton.setPosition({ 530.f, 450.f });

    exitText = std::make_unique<sf::Text>(font, "MAIN MENU", 25);
    exitText->setFillColor(sf::Color::White);
    exitText->setPosition({ 570.f, 465.f });
}

void LoseState::onExit() {
    std::cout << "--- DANG ROI LOSE STATE ---" << std::endl;
}

void LoseState::handleEvent(const sf::Event& event) {
    if (!initialized) return;

    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mousePressed->button == sf::Mouse::Button::Left) {
            sf::Vector2i mousePos = mousePressed->position;

            // Click nút "Try Again"
            if (restartButton.getGlobalBounds().contains(sf::Vector2f(mousePos))) {
                std::cout << "LoseState: Try Again clicked!" << std::endl;
                // TODO: stateMachine.changeState(std::make_unique<PlayState>(stateMachine));
            }

            // Click nút "Main Menu"
            if (exitButton.getGlobalBounds().contains(sf::Vector2f(mousePos))) {
                std::cout << "LoseState: Menu clicked! Quay ve MenuState..." << std::endl;
                stateMachine.changeState(std::make_unique<MenuState>(stateMachine));
            }
        }
    }
}

void LoseState::update(float dt) {}

void LoseState::render(sf::RenderWindow& window) {
    if (!initialized) return;

    window.draw(*loseText);
    window.draw(restartButton);
    window.draw(*restartText);
    window.draw(exitButton);
    window.draw(*exitText);
}