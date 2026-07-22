#pragma once
#include <SFML/Graphics.hpp>
#include "StateMachine.h"
#include "TextureManager.h"

class Game {
public:
    Game();
    ~Game() ; 
    void run();

private:
    TextureManager textureManager;
    sf::RenderWindow window;
    StateMachine stateMachine;
    sf::Clock clock;
};