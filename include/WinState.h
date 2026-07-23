#pragma once
#include "State.h"
#include "StateMachine.h"
#include <SFML/Graphics.hpp>
#include <memory>

class WinState : public State {
private:
    StateMachine& stateMachine;
    sf::Font font;
    bool initialized = false;

    // Các đối tượng chữ (dùng unique_ptr để tránh lỗi khởi tạo font trống)
    std::unique_ptr<sf::Text> winText;
    std::unique_ptr<sf::Text> playAgainText;
    std::unique_ptr<sf::Text> exitText;

    // Các nút bấm hình chữ nhật (biến thông thường)
    sf::RectangleShape playAgainButton;
    sf::RectangleShape exitButton;

public:
    WinState(StateMachine& machine);
    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};