#pragma once

#include <iostream>
#include <vector>

#include "State.h"
#include "Player.h"
#include "Monster.h"
#include "Entity.h"

class GameContext ; //foward declaration cho GameContext

class GameplayState : public State {
private:
    Player player;
    std :: vector<Monster*> monsters ;
    GameContext context ; 

    //bien theo doi vi tri cua chuot
    sf::Vector2f mousePos ; 
    sf :: RenderWindow& window ; 


public:

    GameplayState(sf :: RenderWindow& window) ;
    ~GameplayState() override ;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};