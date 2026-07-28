#include <iostream>
#include <filesystem>
#include "IntroState.h"
#include "MenuState.h"
#include "StateMachine.h"
#include "TextureManager.h"

IntroState::IntroState(StateMachine& machine, sf::RenderWindow& window, TextureManager& textureManager)
    : machine(machine), window(window), textureManager(textureManager), displayTime(3.0f), isDone(false)
{
    //background.setSize(sf::Vector2f(1280.0f, 720.0f));
    //background.setFillColor(sf::Color::Blue);
}

void IntroState::onEnter() {
    // Lấy ảnh nền từ TextureManager
    background = std::make_unique<sf::Sprite>(
        TextureManager::getInstance().getTexture("intro")
    );

    background->setPosition({ 0.f, 0.f });

    auto size = background->getTexture().getSize();

    background->setScale({
        1280.f / size.x,
        720.f / size.y
        });

    std::cout << "--- DANG KHOI DONG INTRO STATE ---" << std::endl;
    std::cout << "Thu muc lam viec hien tai: " << std::filesystem::current_path() << std::endl;

    std::string fontPath = "C:/Project GAME/Eternal-Siege/out/build/x64-debug/assets/fonts/Font.ttf";
    if (!font.openFromFile(fontPath)) {
        std::cout << "KHONG THE NAP FONT!" << std::endl;
        gameTitle.reset();
    }
    else {
        std::cout << "=> NAP FONT THANH CONG!" << std::endl;
        gameTitle = std::make_unique<sf::Text>(font);
        gameTitle->setString("From nowhere of the universe");
        gameTitle->setCharacterSize(50);
        gameTitle->setFillColor(sf::Color::White);

        sf::FloatRect textBounds = gameTitle->getLocalBounds();
        gameTitle->setOrigin({ textBounds.position.x + textBounds.size.x / 2.0f,
                              textBounds.position.y + textBounds.size.y / 2.0f });
        gameTitle->setPosition({ 640.0f, 360.0f });
    }

    displayTime = 3.0f;
    isDone = false;

    std::string path = "C:/Project GAME/Eternal-Siege/out/build/x64-debug/assets/fonts/Font.ttf";
    if (std::filesystem::exists(path)) {
        std::cout << "File exists!" << std::endl;
    }
    else {
        std::cout << "File does not exist!" << std::endl;
    }
}

void IntroState::onExit() {
    std::cout << "--- DANG ROI INTRO STATE ---" << std::endl;
}

void IntroState::handleEvent(const sf::Event& event) {
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::Space ||
            keyPressed->code == sf::Keyboard::Key::Escape) {
            isDone = true;
        }
    }
}

void IntroState::update(float dt) {
    if (!isDone) {
        displayTime -= dt;

        if (displayTime <= 0.0f) {
            isDone = true;
        }
    }

    if (isDone) {
        std::cout << "Chuyen sang MenuState!" << std::endl;
        auto nextState = std::make_unique<MenuState>(machine, window, textureManager);
        machine.changeState(std::move(nextState));
        return;
    }
}

void IntroState::render(sf::RenderWindow& window) {
    // 1. Vẽ hình chữ nhật nền xanh trước
    //window.draw(background);

    //Vẽ background
    if (background)
    {
        window.draw(*background);
    }

    // 2. Vẽ dòng chữ đè lên trên nền (Nhớ kiểm tra pointer để tránh crash nếu nạp font lỗi)
    if (gameTitle) {
        window.draw(*gameTitle);
    }
}