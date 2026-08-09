#pragma once
#include <SFML/Graphics.hpp>
#include "StateMachine.h"
#include "TextureManager.h"
#include "AudioManager.h"
#include "GameProgress.h"

class Game {
public:
    Game();
    ~Game() ; 
    void run();

private:
    TextureManager textureManager;
    AudioManager audioManager;
    GameProgress progress;
    sf::RenderWindow window;
    StateMachine stateMachine;
    sf::Clock clock;
};
