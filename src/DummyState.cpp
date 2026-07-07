#include "DummyState.h"

void DummyState::onEnter() {
    box.setSize({ 100.f, 100.f });
    box.setFillColor(sf::Color::Red);
    box.setPosition({ 300.f, 300.f });
}

void DummyState::onExit() {}

void DummyState::handleEvent(const sf::Event& event) {}

void DummyState::update(float dt) {}

void DummyState::render(sf::RenderWindow& window) {
    window.draw(box);
}