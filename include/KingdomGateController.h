#pragma once

#include <algorithm>

enum class GateState { Closed, Opening, Open };

class KingdomGateController {
public:
    static constexpr float OPEN_DURATION = 0.9f;

    explicit KingdomGateController(bool alreadyOpen = false)
        : state(alreadyOpen ? GateState::Open : GateState::Closed),
          progress(alreadyOpen ? 1.f : 0.f) {}

    bool tryOpen(bool playerInZone) {
        if (!playerInZone || state != GateState::Closed) return false;
        state = GateState::Opening;
        return true;
    }

    void update(float dt) {
        if (state != GateState::Opening) return;
        progress = std::min(1.f, progress + dt / OPEN_DURATION);
        if (progress >= 1.f) state = GateState::Open;
    }

    GateState getState() const { return state; }
    float openProgress() const { return progress; }
    bool blocksPassage() const { return state != GateState::Open; }

private:
    GateState state;
    float progress = 0.f;
};
