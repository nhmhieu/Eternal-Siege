#include "PlayerActionController.h"
#include <cassert>
#include <cmath>

int main() {
    PlayerActionController actions;
    actions.pressDash();
    assert(actions.consumeRequest() == PlayerActionRequest::Dash);
    assert(actions.consumeRequest() == PlayerActionRequest::None);
    actions.pressDash();
    assert(actions.consumeRequest() == PlayerActionRequest::None);
    actions.update(0.8f);
    actions.pressPrimary(); actions.update(0.219f); actions.releasePrimary();
    assert(actions.consumeRequest() == PlayerActionRequest::BasicAttack);
    assert(actions.consumeRequest() == PlayerActionRequest::None);
    actions.update(0.8f);
    actions.pressPrimary(); actions.update(0.22f);
    assert(actions.isCharging());
    assert(actions.consumeRequest() == PlayerActionRequest::None);
    actions.releasePrimary();
    assert(actions.consumeRequest() == PlayerActionRequest::ReleaseHeavy);
    actions.pressPrimary(); actions.update(5.f);
    assert(std::abs(actions.getChargeRatio() - 1.f) < 0.0001f);
    assert(actions.consumeRequest() == PlayerActionRequest::None);
    actions.update(0.8f); actions.pressDash();
    assert(!actions.isCharging());
    assert(actions.consumeRequest() == PlayerActionRequest::Dash);
    actions.releasePrimary();
    assert(actions.consumeRequest() == PlayerActionRequest::None);
    actions.reset(); actions.pressPrimary(); actions.update(0.3f, true);
    assert(!actions.isCharging());
    actions.cancel(); actions.releasePrimary();
    assert(actions.consumeRequest() == PlayerActionRequest::None);
    return 0;
}
