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
    if (auto* cur = getCurrentState()) cur->handleEvent(event);
}

void StateMachine::update(float dt) {
    if (auto* cur = getCurrentState()) cur->update(dt);
}

void StateMachine::render(sf::RenderWindow& window) {
    if (auto* cur = getCurrentState()) cur->render(window);
}