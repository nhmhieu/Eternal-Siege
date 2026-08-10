#pragma once
#include <memory>
#include <stack>
#include "State.h"

class StateMachine {
public:
    void pushState(std::unique_ptr<State> state);
    void popState();
    void changeState(std::unique_ptr<State> state);
    State* getCurrentState();

    void handleEvent(const sf::Event& event);
    void update(float dt);
    void render(sf::RenderWindow& window);
    bool consumeTransitionApplied();

private:
    std::stack<std::unique_ptr<State>> states;
    std::unique_ptr<State> pendingState;
    bool isDispatching = false;
    bool transitionApplied = false;

    void applyPendingState();
};
