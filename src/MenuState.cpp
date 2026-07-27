#include "MenuState.h"
#include "SetupState.h"
#include <iostream>
#include <fstream>
#include <vector>

// Constructor bây giờ rất sạch sẽ, không gọi font rỗng nữa
MenuState::MenuState(StateMachine& machine, sf::RenderWindow& window)
    : stateMachine(machine), window(window) {
}

static std::vector<char> fontBuffer;

void MenuState::onEnter() {
    if (!font.openFromFile("assets/fonts/Font.ttf")){
        std::cerr << "Failed to load font!" << std::endl;
        return;
    }

    initialized = true;
    std::cout << "=> KHONG CO GI NGAN CAN DUOC NUA! NAP FONT THANH CONG!" << std::endl;
    
    // BƯỚC 2: Font đã nạp xong, giờ mới an toàn tạo các đối tượng sf::Text
    titleText = std::make_unique<sf::Text>(font, "ETERNAL SIEGE", 98);
    titleText->setFillColor(sf::Color::Yellow);
    titleText->setPosition({ 300.f, 150.f });

    startText = std::make_unique<sf::Text>(font, "BAT DAU", 30);
    startText->setFillColor(sf::Color::White);
    startText->setPosition({ 595.f, 360.f });

    exitText = std::make_unique<sf::Text>(font, "THOAT", 30);
    exitText->setFillColor(sf::Color::White);
    exitText->setPosition({ 605.f, 460.f });

    // Các nút bấm hình chữ nhật không cần font
    startButton = std::make_unique<sf::RectangleShape>();
    startButton->setSize(sf::Vector2f(200.f, 60.f));
    startButton->setFillColor(sf::Color(50, 150, 50));
    startButton->setPosition({ 540.f, 350.f });

    exitButton = std::make_unique<sf::RectangleShape>();
    exitButton->setSize(sf::Vector2f(200.f, 60.f));
    exitButton->setFillColor(sf::Color(150, 50, 50));
    exitButton->setPosition({ 540.f, 450.f });
}

void MenuState::onExit() {
    // Các unique_ptr sẽ tự động dọn dẹp một cách an toàn
}

void MenuState::handleEvent(const sf::Event& event) {
    if (!initialized) return;

    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mousePressed->button == sf::Mouse::Button::Left) {
            sf::Vector2i mousePos = mousePressed->position;

            // Dùng dấu -> vì startButton giờ là biến thông thường
            if (startButton && startButton->getGlobalBounds().contains(sf::Vector2f(mousePos))) {
                std::cout << "Start button clicked!" << std::endl;
                stateMachine.changeState(std::make_unique<SetupState>(stateMachine, window));
                return;
            }

            if (exitButton && exitButton->getGlobalBounds().contains(sf::Vector2f(mousePos))) {
                std::cout << "Exit button clicked!" << std::endl;
                window.close();
            }
        }
    }
}

void MenuState::update(float dt) {}

void MenuState::render(sf::RenderWindow& window) {
    if (!initialized) return;

    // Dùng dấu * để giải băm unique_ptr khi vẽ
   if (titleText) window.draw(*titleText);
   if (startButton) window.draw(*startButton);
   if (startText) window.draw(*startText);
   if (exitButton) window.draw(*exitButton);
   if (exitText) window.draw(*exitText);
}