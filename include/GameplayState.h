#pragma once

#include <iostream>
#include <vector>

#include "State.h"
#include "Player.h"
#include "Monster.h"
#include "Entity.h"
#include "CombatManager.h"

class GameContext ; //foward declaration cho GameContext

class GameplayState : public State {
private:
    //Player ingame
    Player player;

    std :: vector<Entity *> monsters ;
    std :: vector<Entity*> entities ; 


    // cai nay le ra la cai chua moi thong tin cua game
    GameContext context ; 

    //bien theo doi vi tri cua chuot
    sf::Vector2f mousePos ; 
    sf :: RenderWindow& window ; 

    //Xu li combat
    CombatManager combatManager ; 


public:

    GameplayState(sf :: RenderWindow& window) ;
    ~GameplayState() override ;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};