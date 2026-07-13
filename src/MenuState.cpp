#include "MenuState.h"
#include <iostream>

MenuState::MenuState(StateMachine& machine)
    : stateMachine(machine), initialized(false), titleText(font), startText(font), exitText(font) {
}

void MenuState::onEnter() {
    // Sửa đường dẫn tuyệt đối tạm thời để đảm bảo nạp font chạy được luôn trên máy bạn
    if (!font.openFromFile("assets/Montserrat-Italic.ttf")) {
        std::cerr << "Failed to load font trong MenuState!" << std::endl;
        initialized = false;
        return;
    }
    initialized = true;

    titleText.setString("ETERNAL SIEGE");
    titleText.setCharacterSize(68);
    titleText.setFillColor(sf::Color::Yellow);
    titleText.setPosition({ 300.f, 150.f });

    startButton.setSize(sf::Vector2f(200.f, 60.f));
    startButton.setFillColor(sf::Color(50, 150, 50));
    startButton.setPosition({ 540.f, 350.f });

    startText.setString("BAT DAU");
    startText.setCharacterSize(30);
    startText.setFillColor(sf::Color::White);
    startText.setPosition({ 595.f, 360.f });

    exitButton.setSize(sf::Vector2f(200.f, 60.f));
    exitButton.setFillColor(sf::Color(150, 50, 50));
    exitButton.setPosition({ 540.f, 450.f });

    exitText.setString("THOAT");
    exitText.setCharacterSize(30);
    exitText.setFillColor(sf::Color::White);
    exitText.setPosition({ 605.f, 460.f });
}

void MenuState::onExit() {
    // Đối tượng thông thường tự giải phóng khi MenuState bị hủy, không lo leak bộ nhớ
}

void MenuState::handleEvent(const sf::Event& event) {
    // Nếu chưa khởi tạo thành công thì không xử lý click nút để tránh lỗi logic
    if (!initialized) return;

    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mousePressed->button == sf::Mouse::Button::Left) {
            sf::Vector2i mousePos = mousePressed->position;

            if (startButton.getGlobalBounds().contains(sf::Vector2f(mousePos))) {
                std::cout << "Start button clicked!" << std::endl;
            }

            if (exitButton.getGlobalBounds().contains(sf::Vector2f(mousePos))) {
                std::cout << "Exit button clicked!" << std::endl;
                // Thêm logic đóng cửa sổ nếu cần ở đây
            }
        }
    }
}

void MenuState::update(float dt) {}

void MenuState::render(sf::RenderWindow& window) {
    // Chốt chặn an toàn: Chỉ vẽ khi toàn bộ text và button đã được nạp thành công
    if (!initialized) return;

    window.draw(titleText);
    window.draw(startButton);
    window.draw(startText);
    window.draw(exitButton);
    window.draw(exitText);
}