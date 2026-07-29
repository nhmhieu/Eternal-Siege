#include "StateMachine.h"

void StateMachine::pushState(std::unique_ptr<State> state) {
    if (!states.empty()) states.top()->onExit();
    state->onEnter();
    states.push(std::move(state));
}

void StateMachine::popState() {
    if (!states.empty()) {
        states.top()->onExit();
        states.pop();
        if (!states.empty()) states.top()->onEnter();
    }
}

void StateMachine::changeState(std::unique_ptr<State> state) {
    if (isDispatching) {
        pendingState = std::move(state);
        return;
    }

    if (!states.empty()) {
        states.top()->onExit();
        states.pop();
    }
    state->onEnter();
    states.push(std::move(state));
}

State* StateMachine::getCurrentState() {
    return states.empty() ? nullptr : states.top().get();
}

void StateMachine::handleEvent(const sf::Event& event) {
    isDispatching = true;
    if (auto* cur = getCurrentState()) cur->handleEvent(event);
    isDispatching = false;
    applyPendingState();
}

void StateMachine::update(float dt) {
    isDispatching = true;
    if (auto* cur = getCurrentState()) cur->update(dt);
    isDispatching = false;
    applyPendingState();
}

void StateMachine::render(sf::RenderWindow& window) {
    if (auto* cur = getCurrentState()) cur->render(window);
}

void StateMachine::applyPendingState() {
    if (!pendingState) return;
    auto nextState = std::move(pendingState);
    changeState(std::move(nextState));
}
