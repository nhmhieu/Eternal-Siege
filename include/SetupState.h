#pragma once

#include "State.h"
#include "Map.h"
#include "StateMachine.h"

class SetupState : public State
{
private:
    Map map;
    StateMachine& stateMachine;

public:
    SetupState(StateMachine& sm);
    ~SetupState() override = default;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};