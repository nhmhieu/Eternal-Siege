#pragma once

enum class GameplayEndState {
    None,
    GameOver,
    Victory
};

class GameplayTransitionGate {
public:
    void reset() {
        requested = GameplayEndState::None;
        acceptedCount = 0;
    }

    bool request(GameplayEndState next) {
        if (next == GameplayEndState::None || hasRequest()) return false;
        requested = next;
        ++acceptedCount;
        return true;
    }

    bool hasRequest() const {
        return requested != GameplayEndState::None;
    }

    GameplayEndState getRequestedState() const { return requested; }
    int getAcceptedCount() const { return acceptedCount; }

private:
    GameplayEndState requested = GameplayEndState::None;
    int acceptedCount = 0;
};
