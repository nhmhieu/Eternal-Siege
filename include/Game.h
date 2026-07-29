#pragma once
#include <SFML/Graphics.hpp>
#include "StateMachine.h"
#include <iostream>
#include "TextureManager.h"


class Game {
public:
    Game();
    ~Game() ; 
    void run();

private:
    sf::RenderWindow window;
    StateMachine stateMachine;
    sf::Clock clock;

    
};