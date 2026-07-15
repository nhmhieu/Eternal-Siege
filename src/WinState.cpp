#include "WinState.h"
#include "MenuState.h" // Để quay lại Menu chính khi bấm thoát hoặc chơi lại tùy logic
#include <iostream>

WinState::WinState(StateMachine& machine)
    : stateMachine(machine), initialized(false) {
}

void WinState::onEnter() {
    std::cout << "--- DANG MO WIN STATE ---" << std::endl;

    // Nạp font tương đối (tạm thời không load được cũng không sao nhờ chốt chặn initialized)
    if (!font.openFromFile("assets/Montserrat-Italic.ttf")) {
        std::cerr << "WinState: Failed to load font!" << std::endl;
        initialized = false;
        return;
    }
    initialized = true;

    // 1. Dòng chữ Chiến Thắng (Màu xanh lá cây/vàng rực rỡ)
    winText = std::make_unique<sf::Text>(font, "YOU WIN!", 80);
    winText->setFillColor(sf::Color::Green);
    winText->setPosition({ 440.f, 150.f });

    // 2. Nút Chơi Tiếp / Chơi Lại
    playAgainButton.setSize(sf::Vector2f(220.f, 60.f));
    playAgainButton.setFillColor(sf::Color(50, 150, 50));
    playAgainButton.setPosition({ 530.f, 350.f });

    playAgainText = std::make_unique<sf::Text>(font, "PLAY AGAIN", 25);
    playAgainText->setFillColor(sf::Color::White);
    playAgainText->setPosition({ 565.f, 365.f });

    // 3. Nút Thoát (Về Menu chính hoặc đóng game)
    exitButton.setSize(sf::Vector2f(220.f, 60.f));
    exitButton.setFillColor(sf::Color(150, 50, 50));
    exitButton.setPosition({ 530.f, 450.f });

    exitText = std::make_unique<sf::Text>(font, "MAIN MENU", 25);
    exitText->setFillColor(sf::Color::White);
    exitText->setPosition({ 570.f, 465.f });
}

void WinState::onExit() {
    std::cout << "--- DANG ROI WIN STATE ---" << std::endl;
}

void WinState::handleEvent(const sf::Event& event) {
    if (!initialized) return;

    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mousePressed->button == sf::Mouse::Button::Left) {
            sf::Vector2i mousePos = mousePressed->position;

            // Xử lý click nút "Play Again"
            if (playAgainButton.getGlobalBounds().contains(sf::Vector2f(mousePos))) {
                std::cout << "WinState: Play Again clicked! Chuyen sang PlayState..." << std::endl;
                // TODO: stateMachine.changeState(std::make_unique<PlayState>(stateMachine));
            }

            // Xử lý click nút "Main Menu"
            if (exitButton.getGlobalBounds().contains(sf::Vector2f(mousePos))) {
                std::cout << "WinState: Menu clicked! Quay lai MenuState..." << std::endl;
                stateMachine.changeState(std::make_unique<MenuState>(stateMachine));
            }
        }
    }
}

void WinState::update(float dt) {}

void WinState::render(sf::RenderWindow& window) {
    if (!initialized) return;

    window.draw(*winText);
    window.draw(playAgainButton);
    window.draw(*playAgainText);
    window.draw(exitButton);
    window.draw(*exitText);
}