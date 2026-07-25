#include "SetupState.h"
#include "GameplayState.h" // <-- 1. ĐÃ THÊM DÒNG NÀY ĐỂ SỬA LỖI UNDEFINED
#include <iostream>

SetupState::SetupState(StateMachine& machine)
    : stateMachine(machine), map(15, 15)
{
}

void SetupState::onEnter()
{
    if (!font.openFromFile("assets/fonts/Roboto-Regular.ttf"))
    {
        std::cerr << "Failed to load font!" << std::endl;
    }

    startButton.setSize(sf::Vector2f(200.f, 60.f));
    startButton.setFillColor(sf::Color::Green);
    startButton.setPosition(sf::Vector2f(540.f, 600.f));

    // 2. ĐÃ XÓA ĐOẠN KHỞI TẠO BỊ TRÙNG LẶP
    startText = std::make_unique<sf::Text>(font);
    startText->setString("BAT DAU");
    startText->setCharacterSize(30);
    startText->setFillColor(sf::Color::White);

    auto bounds = startText->getLocalBounds();

    startText->setOrigin(
    {
        bounds.position.x + bounds.size.x / 2.f,
        bounds.position.y + bounds.size.y / 2.f
    });

    startText->setPosition(
    {
        startButton.getPosition().x + startButton.getSize().x / 2.f,
        startButton.getPosition().y + startButton.getSize().y / 2.f
    });

    canStart = false;
    selectedPositions.clear();
}

void SetupState::onExit()
{
    std::cout << "Thoat SetupState!" << std::endl;
}

void SetupState::handleEvent(const sf::Event& event)
{
    if (const auto* mousePress = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mousePress->button == sf::Mouse::Button::Left)
        {
            sf::Vector2i mousePos = mousePress->position;

            if (canStart &&
                startButton.getGlobalBounds().contains(sf::Vector2f(mousePos)))
            {
                std::cout << "Bat dau game voi "
                          << selectedPositions.size()
                          << " tuong!" << std::endl;

                // 3. ĐÃ BỎ LỆNH "return;" BỊ CHẶN Ở TRƯỚC ĐỂ GAME CHUYỂN STATE ĐƯỢC
                stateMachine.changeState(
                    std::make_unique<GameplayState>(
                        map,
                        selectedPositions));
                return; // Đặt return ở ĐÂY sau khi đã đổi state (hoặc để trống nếu hàm kết thúc)
            }

            map.handleMouseClick(mousePos.x,
                                 mousePos.y,
                                 selectedPositions,
                                 maxAllies);

            if (static_cast<int>(selectedPositions.size()) == maxAllies)
            {
                canStart = true;
                std::cout << "Da du 4 tuong!" << std::endl;
            }
        }
    }
}

void SetupState::update(float dt)
{
}

void SetupState::render(sf::RenderWindow& window)
{
    map.draw(window);

    if (canStart)
    {
        window.draw(startButton);

        if (startText)
            window.draw(*startText);
    }
}