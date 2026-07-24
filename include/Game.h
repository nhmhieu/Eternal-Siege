#pragma once
#include <SFML/Graphics.hpp>
#include "StateMachine.h"
#include <iostream>


class Game {
public:
    Game();
    void run();

private:
    sf::RenderWindow window;
    StateMachine stateMachine;
    sf::Clock clock;

    
};