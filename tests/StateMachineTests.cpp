#include "StateMachine.h"

#include <cassert>
#include <memory>

namespace {
class TestState final : public State {
public:
    TestState(
        StateMachine& owner,
        int stateId,
        bool redirectOnEnter = false,
        int redirectId = 0
    )
        : machine(owner),
          id(stateId),
          shouldRedirectOnEnter(redirectOnEnter),
          nextId(redirectId) {
    }

    ~TestState() override {
        assert(!insideCallback);
    }

    void onEnter() override {
        insideCallback = true;
        if (shouldRedirectOnEnter) {
            machine.changeState(
                std::make_unique<TestState>(machine, nextId));
        }
        insideCallback = false;
    }

    void onExit() override {
        insideCallback = true;
        insideCallback = false;
    }

    void handleEvent(const sf::Event&) override {
    }

    void update(float) override {
        insideCallback = true;
        if (redirectOnUpdate) {
            redirectOnUpdate = false;
            machine.changeState(
                std::make_unique<TestState>(machine, nextId));
        }
        insideCallback = false;
    }

    void render(sf::RenderWindow&) override {
    }

    int getId() const {
        return id;
    }

    void redirectDuringNextUpdate(int destinationId) {
        redirectOnUpdate = true;
        nextId = destinationId;
    }

private:
    StateMachine& machine;
    int id;
    bool shouldRedirectOnEnter = false;
    bool redirectOnUpdate = false;
    int nextId = 0;
    bool insideCallback = false;
};

TestState* currentTestState(StateMachine& machine) {
    return dynamic_cast<TestState*>(machine.getCurrentState());
}
}

int main() {
    StateMachine machine;

    // A transition requested by onEnter must be deferred until the callback
    // returns, otherwise the currently executing object is destroyed.
    machine.changeState(
        std::make_unique<TestState>(machine, 1, true, 2));
    assert(currentTestState(machine));
    assert(currentTestState(machine)->getId() == 2);

    // The same lifetime guarantee is required during regular update dispatch.
    currentTestState(machine)->redirectDuringNextUpdate(3);
    machine.update(0.016f);
    assert(currentTestState(machine));
    assert(currentTestState(machine)->getId() == 3);

    machine.popState();
    assert(machine.getCurrentState() == nullptr);
    return 0;
}
