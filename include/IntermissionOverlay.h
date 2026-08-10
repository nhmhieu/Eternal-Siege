#pragma once

#include "UpgradeManager.h"

#include <SFML/Graphics.hpp>
#include <cstddef>
#include <string_view>

enum class IntermissionPromptState {
    Inactive,
    Normal,
    AwaitingSkipConfirmation
};

enum class IntermissionAction {
    Ignored,
    Consumed,
    ShowSkipWarning,
    BeginNextWave
};

enum class IntermissionFeedback {
    None,
    DamageUpgraded,
    VitalityUpgraded,
    FireRateUpgraded,
    NotEnoughGold,
    MaxLevel,
    UpgradeUndone,
    NothingToUndo
};

class IntermissionController {
public:
    void resetForNewGame();
    void sync(bool intermission, int completedWave);
    IntermissionAction handleEnter(std::size_t retainedUpgradeCount);
    bool handleEnterReleased();
    void cancelSkipConfirmation();
    void notifyPurchase(UpgradeType type, bool success,
                        bool maxLevel = false);
    void notifyUndo(bool success);
    void update(float deltaTime, bool paused);

    bool isActive() const { return active; }
    bool isFirstGuidedIntermission() const { return guidedFirst; }
    int getCompletedWave() const { return completedWave; }
    IntermissionPromptState getPromptState() const { return promptState; }
    IntermissionFeedback getFeedback() const { return feedback; }
    int getHighlightedUpgrade() const { return highlightedUpgrade; }
    float getFeedbackRemaining() const { return feedbackRemaining; }

private:
    bool active = false;
    bool firstIntermissionSeen = false;
    bool guidedFirst = false;
    bool enterHeld = false;
    int completedWave = 0;
    int highlightedUpgrade = -1;
    float feedbackRemaining = 0.f;
    IntermissionPromptState promptState = IntermissionPromptState::Inactive;
    IntermissionFeedback feedback = IntermissionFeedback::None;
};

class IntermissionOverlay {
public:
    bool load();
    void draw(sf::RenderWindow& target,
              const IntermissionController& controller,
              const UpgradeManager& upgrades) const;

private:
    sf::Font font;
    bool fontLoaded = false;
};

std::string_view intermissionFeedbackText(IntermissionFeedback feedback);
