#pragma once

#include <iostream>
#include <vector>

#include "State.h"
#include "Player.h"
#include "Monster.h"
#include "Entity.h"

class GameplayState : public State {
private:
    Player player;
    std :: vector<Monster*> monsters ;
    std :: vector<Entity*> tempEntity ; 

public:
    GameplayState() ;
    ~GameplayState() override ;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};