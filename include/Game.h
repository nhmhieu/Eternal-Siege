#pragma once
#include <SFML/Graphics.hpp>
#include "StateMachine.h"
#include "TextureManager.h"
#include "AudioManager.h"

class Game {
public:
    Game();
    ~Game() ; 
    void run();

private:
    TextureManager textureManager;
    AudioManager audioManager;
    sf::RenderWindow window;
    StateMachine stateMachine;
    sf::Clock clock;
};
