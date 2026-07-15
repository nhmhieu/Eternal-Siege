#pragma once
#include "State.h"
#include "TextureManager.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <iostream>

class CharacterState : public State {
private:
    TextureManager& textureManager;
    //sf::Sprite box;
    std::unique_ptr<sf::Sprite> box;

public:
    CharacterState(TextureManager& textureManager);

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};