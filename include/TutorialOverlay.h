#pragma once

#include <SFML/Graphics.hpp>

enum class TutorialMode {
    Hidden,
    FirstLaunch,
    Reopened
};

enum class TutorialKey {
    Enter,
    Help
};

enum class TutorialTransition {
    Ignored,
    Consumed,
    InitialClosed,
    Reopened,
    Closed
};

class TutorialController {
public:
    void resetForNewGame();
    TutorialTransition handleKeyPressed(TutorialKey key);
    bool handleKeyReleased(TutorialKey key);

    TutorialMode getMode() const { return mode; }
    bool isVisible() const { return mode != TutorialMode::Hidden; }
    bool blocksGameplayUpdate() const {
        return isVisible() || awaitingInputRelease;
    }
    bool blocksGameplayInput() const {
        return isVisible() || awaitingInputRelease;
    }
    bool isAwaitingInputRelease() const { return awaitingInputRelease; }
    bool hasModalKeyHeld() const { return enterHeld || helpHeld; }
    void finishInputRelease() { awaitingInputRelease = false; }

private:
    TutorialMode mode = TutorialMode::FirstLaunch;
    bool enterHeld = false;
    bool helpHeld = false;
    bool awaitingInputRelease = false;
};

class TutorialOverlay {
public:
    bool load();
    void draw(sf::RenderWindow& target, TutorialMode mode) const;

private:
    sf::Font font;
    bool fontLoaded = false;
};
