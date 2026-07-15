#include "CharacterState.h"

CharacterState::CharacterState(TextureManager& textureManager)
    : textureManager(textureManager) {}

void CharacterState::onEnter() {

    // Lấy texture đã load trong Game
    //box.setTexture(textureManager.getTexture("player"));
    box = std::make_unique<sf::Sprite>(
        textureManager.getTexture("player"));

    //box.setSize({ 100.f, 100.f });
    //box.setFillColor(sf::Color::Red);
    //box.setPosition({ 300.f, 300.f });
    box->setPosition({ 300.f, 300.f });
}

void CharacterState::onExit() {}

void CharacterState::handleEvent(const sf::Event& event) {}

void CharacterState::update(float dt) {}

void CharacterState::render(sf::RenderWindow& window) {
    //window.draw(box);
    if (box)
        //std::cout << "Drawing sprite\n";
        window.draw(*box);
}