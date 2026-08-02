#include "MenuState.h"
#include "SetupState.h"
#include "AssetLocator.h"
#include <algorithm>
#include <iostream>

// Constructor bây giờ rất sạch sẽ, không gọi font rỗng nữa
MenuState::MenuState(StateMachine& machine, sf::RenderWindow& window,
                     TextureManager& textureManager, AudioManager& audioManager)
    : stateMachine(machine), window(window), textureManager(textureManager),
      audioManager(audioManager) {
}


void MenuState::onEnter() {
    audioManager.playMusic("assets/audio/music/menu_theme.ogg");
    const auto fontPath = AssetLocator::find("assets/fonts/Font.ttf");
    if (!fontPath || !font.openFromFile(*fontPath)){
        std::cerr << "Failed to load font!" << std::endl;
        return;
    }

    
    // BƯỚC 2: Font đã nạp xong, giờ mới an toàn tạo các đối tượng sf::Text
    titleText = std::make_unique<sf::Text>(font, "ETERNAL SIEGE", 98);
    titleText->setFillColor(sf::Color::Yellow);
    titleText->setPosition({ 300.f, 150.f });

    if (textureManager.loadTexture("Logo", "assets/images/ui/logo.png")) {
        if (const sf::Texture* logo = textureManager.findTexture("Logo")) {
            logoSprite.emplace(*logo);
            const sf::Vector2u size = logo->getSize();
            const float scale = std::min(600.f / static_cast<float>(size.x),
                                         225.f / static_cast<float>(size.y));
            logoSprite->setScale({scale, scale});
            logoSprite->setOrigin({static_cast<float>(size.x) / 2.f,
                                   static_cast<float>(size.y) / 2.f});
            logoSprite->setPosition({640.f, 210.f});
            titleText.reset();
        }
    }

    startText = std::make_unique<sf::Text>(font, "START", 25);
    startText->setFillColor(sf::Color::White);
    auto bounds = startText->getLocalBounds();
    startText->setOrigin({bounds.position.x + bounds.size.x / 2.f,
                          bounds.position.y + bounds.size.y / 2.f});
    startText->setPosition({640.f, 383.f});

    exitText = std::make_unique<sf::Text>(font, "EXIT", 25);
    exitText->setFillColor(sf::Color::White);
    bounds = exitText->getLocalBounds();
    exitText->setOrigin({bounds.position.x + bounds.size.x / 2.f,
                         bounds.position.y + bounds.size.y / 2.f});
    exitText->setPosition({640.f, 473.f});

    // Các nút bấm hình chữ nhật không cần font
    startButton = std::make_unique<sf::RectangleShape>();
    startButton->setSize(sf::Vector2f(250.f, 64.f));
    startButton->setFillColor(sf::Color(18, 31, 43, 238));
    startButton->setOutlineColor(sf::Color(215, 166, 80));
    startButton->setOutlineThickness(2.f);
    startButton->setPosition({515.f, 350.f});

    exitButton = std::make_unique<sf::RectangleShape>();
    exitButton->setSize(sf::Vector2f(250.f, 64.f));
    exitButton->setFillColor(sf::Color(18, 31, 43, 238));
    exitButton->setOutlineColor(sf::Color(130, 111, 104));
    exitButton->setOutlineThickness(2.f);
    exitButton->setPosition({515.f, 440.f});
}

void MenuState::onExit() {
    // Các unique_ptr sẽ tự động dọn dẹp một cách an toàn
}

void MenuState::handleEvent(const sf::Event& event) {

    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mousePressed->button == sf::Mouse::Button::Left) {
            sf::Vector2i mousePos = mousePressed->position;

            // Dùng dấu -> vì startButton giờ là biến thông thường
            if (startButton && startButton->getGlobalBounds().contains(sf::Vector2f(mousePos))) {
                audioManager.playSound("ui_click");
                stateMachine.changeState(std::make_unique<SetupState>(
                    stateMachine, window, textureManager, audioManager));
                return;
            }

            if (exitButton && exitButton->getGlobalBounds().contains(sf::Vector2f(mousePos))) {
                audioManager.playSound("ui_click");
                window.close();
            }
        }
    }
}

void MenuState::update(float) {
    if (!startButton || !exitButton) return;
    const sf::Vector2f mouse = window.mapPixelToCoords(
        sf::Mouse::getPosition(window));
    const bool startHover = startButton->getGlobalBounds().contains(mouse);
    const bool exitHover = exitButton->getGlobalBounds().contains(mouse);
    startButton->setFillColor(startHover
        ? sf::Color(42, 74, 78, 245) : sf::Color(18, 31, 43, 238));
    startButton->setOutlineColor(startHover
        ? sf::Color(255, 218, 132) : sf::Color(215, 166, 80));
    exitButton->setFillColor(exitHover
        ? sf::Color(65, 38, 48, 245) : sf::Color(18, 31, 43, 238));
    exitButton->setOutlineColor(exitHover
        ? sf::Color(225, 108, 108) : sf::Color(130, 111, 104));
}

void MenuState::render(sf::RenderWindow& window) {
   window.clear(sf::Color(7, 12, 20));
   sf::RectangleShape panel({390.f, 245.f});
   panel.setOrigin(panel.getSize() / 2.f);
   panel.setPosition({640.f, 427.f});
   panel.setFillColor(sf::Color(10, 20, 31, 220));
   panel.setOutlineColor(sf::Color(155, 117, 61, 150));
   panel.setOutlineThickness(1.5f);
   window.draw(panel);

    // Dùng dấu * để giải băm unique_ptr khi vẽ
   if (logoSprite) window.draw(*logoSprite);
   if (titleText) window.draw(*titleText);
   if (startButton) window.draw(*startButton);
   if (startText) window.draw(*startText);
   if (exitButton) window.draw(*exitButton);
   if (exitText) window.draw(*exitText);
}
