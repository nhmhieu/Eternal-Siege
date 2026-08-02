#include "IntermissionOverlay.h"

#include "AssetLocator.h"

#include <algorithm>
#include <array>
#include <string>

namespace {
constexpr float FEEDBACK_DURATION = 1.35f;
int upgradeIndex(UpgradeType type) { return static_cast<int>(type); }
}

void IntermissionController::resetForNewGame() {
    active = false;
    firstIntermissionSeen = false;
    guidedFirst = false;
    enterHeld = false;
    completedWave = 0;
    highlightedUpgrade = -1;
    feedbackRemaining = 0.f;
    promptState = IntermissionPromptState::Inactive;
    feedback = IntermissionFeedback::None;
}

void IntermissionController::sync(bool intermission, int wave) {
    if (!intermission) {
        active = false;
        guidedFirst = false;
        promptState = IntermissionPromptState::Inactive;
        return;
    }
    if (active) return;
    active = true;
    completedWave = wave;
    guidedFirst = !firstIntermissionSeen;
    firstIntermissionSeen = true;
    promptState = IntermissionPromptState::Normal;
    feedback = IntermissionFeedback::None;
    feedbackRemaining = 0.f;
    highlightedUpgrade = -1;
}

IntermissionAction IntermissionController::handleEnter(
    std::size_t retainedUpgradeCount) {
    if (!active) return IntermissionAction::Ignored;
    if (enterHeld) return IntermissionAction::Consumed;
    enterHeld = true;
    if (guidedFirst && retainedUpgradeCount == 0 &&
        promptState != IntermissionPromptState::AwaitingSkipConfirmation) {
        promptState = IntermissionPromptState::AwaitingSkipConfirmation;
        feedback = IntermissionFeedback::None;
        feedbackRemaining = 0.f;
        return IntermissionAction::ShowSkipWarning;
    }
    return IntermissionAction::BeginNextWave;
}

bool IntermissionController::handleEnterReleased() {
    const bool consumed = enterHeld;
    enterHeld = false;
    return consumed;
}

void IntermissionController::cancelSkipConfirmation() {
    if (active) promptState = IntermissionPromptState::Normal;
}

void IntermissionController::notifyPurchase(UpgradeType type, bool success,
                                            bool maxLevel) {
    cancelSkipConfirmation();
    highlightedUpgrade = upgradeIndex(type);
    feedbackRemaining = FEEDBACK_DURATION;
    if (!success) {
        feedback = maxLevel ? IntermissionFeedback::MaxLevel
                            : IntermissionFeedback::NotEnoughGold;
        return;
    }
    switch (type) {
    case UpgradeType::Damage:
        feedback = IntermissionFeedback::DamageUpgraded;
        break;
    case UpgradeType::Vitality:
        feedback = IntermissionFeedback::VitalityUpgraded;
        break;
    case UpgradeType::FireRate:
        feedback = IntermissionFeedback::FireRateUpgraded;
        break;
    }
}

void IntermissionController::notifyUndo(bool success) {
    cancelSkipConfirmation();
    highlightedUpgrade = -1;
    feedbackRemaining = FEEDBACK_DURATION;
    feedback = success ? IntermissionFeedback::UpgradeUndone
                       : IntermissionFeedback::NothingToUndo;
}

void IntermissionController::update(float deltaTime, bool paused) {
    if (paused || feedbackRemaining <= 0.f) return;
    feedbackRemaining = std::max(0.f, feedbackRemaining - deltaTime);
    if (feedbackRemaining <= 0.f) {
        feedback = IntermissionFeedback::None;
        highlightedUpgrade = -1;
    }
}

std::string_view intermissionFeedbackText(IntermissionFeedback feedback) {
    switch (feedback) {
    case IntermissionFeedback::DamageUpgraded: return "Damage upgraded!";
    case IntermissionFeedback::VitalityUpgraded: return "Vitality upgraded!";
    case IntermissionFeedback::FireRateUpgraded: return "Fire Rate upgraded!";
    case IntermissionFeedback::NotEnoughGold: return "Not enough Gold";
    case IntermissionFeedback::MaxLevel: return "Maximum level reached";
    case IntermissionFeedback::UpgradeUndone: return "Last upgrade undone";
    case IntermissionFeedback::NothingToUndo: return "Nothing to undo";
    default: return {};
    }
}

bool IntermissionOverlay::load() {
    const auto path = AssetLocator::find("assets/fonts/Font.ttf");
    fontLoaded = path && font.openFromFile(*path);
    return fontLoaded;
}

void IntermissionOverlay::draw(
    sf::RenderWindow& target, const IntermissionController& controller,
    const UpgradeManager& upgrades) const {
    if (!controller.isActive()) return;
    const sf::View view = target.getView();
    const sf::Vector2f size = view.getSize();
    const sf::Vector2f topLeft = view.getCenter() - size / 2.f;

    sf::RectangleShape veil(size);
    veil.setPosition(topLeft);
    veil.setFillColor(sf::Color(2, 7, 13, 150));
    target.draw(veil);

    const float panelWidth = std::min(
        std::clamp(size.x * 0.85f, 900.f, 1120.f), size.x - 30.f);
    const float panelHeight = std::min(
        std::clamp(size.y * 0.79f, 540.f, 610.f), size.y - 24.f);
    const sf::Vector2f panelPosition{
        view.getCenter().x - panelWidth / 2.f,
        view.getCenter().y - panelHeight / 2.f};
    sf::RectangleShape panel({panelWidth, panelHeight});
    panel.setPosition(panelPosition);
    panel.setFillColor(sf::Color(8, 22, 32, 247));
    panel.setOutlineColor(sf::Color(211, 164, 75, 235));
    panel.setOutlineThickness(3.f);
    target.draw(panel);
    if (!fontLoaded) return;

    const auto drawText = [&](const std::string& value, sf::Vector2f position,
                              unsigned sizePx, sf::Color color,
                              bool bold = false) {
        sf::Text text(font, value, sizePx);
        text.setPosition(position);
        text.setFillColor(color);
        text.setOutlineColor(sf::Color(0, 0, 0, 160));
        text.setOutlineThickness(1.f);
        if (bold) text.setStyle(sf::Text::Bold);
        target.draw(text);
    };

    const sf::Color gold(242, 198, 91);
    const sf::Color teal(89, 229, 202);
    const sf::Color body(227, 235, 237);
    const sf::Color muted(154, 175, 182);
    drawText("WAVE " + std::to_string(controller.getCompletedWave()) +
                 " CLEARED", {panelPosition.x + 34.f, panelPosition.y + 22.f},
             29, gold, true);
    drawText("Available Gold: " + std::to_string(upgrades.getGold()),
             {panelPosition.x + panelWidth - 260.f, panelPosition.y + 29.f},
             19, teal, true);
    if (controller.isFirstGuidedIntermission()) {
        drawText("Spend Gold to strengthen your team before the next wave.",
                 {panelPosition.x + 34.f, panelPosition.y + 72.f},
                 16, body, true);
    }

    constexpr std::array<UpgradeType, 3> types{{
        UpgradeType::Damage, UpgradeType::Vitality, UpgradeType::FireRate}};
    const float gap = 16.f;
    const float cardWidth = (panelWidth - 68.f - gap * 2.f) / 3.f;
    const float cardHeight = 265.f;
    const float cardY = panelPosition.y + 116.f;
    for (std::size_t index = 0; index < types.size(); ++index) {
        const UpgradePreview preview = upgrades.preview(types[index]);
        const float cardX = panelPosition.x + 34.f +
            static_cast<float>(index) * (cardWidth + gap);
        const bool highlighted = controller.getHighlightedUpgrade() ==
                                 static_cast<int>(index) &&
                                 controller.getFeedbackRemaining() > 0.f;
        const bool failure = highlighted &&
            (controller.getFeedback() == IntermissionFeedback::NotEnoughGold ||
             controller.getFeedback() == IntermissionFeedback::MaxLevel);
        sf::RectangleShape card({cardWidth, cardHeight});
        card.setPosition({cardX, cardY});
        card.setFillColor(highlighted
            ? (failure ? sf::Color(72, 30, 30, 245)
                       : sf::Color(27, 69, 61, 245))
            : sf::Color(16, 34, 45, 244));
        card.setOutlineColor(highlighted
            ? (failure ? sf::Color(238, 104, 78)
                       : sf::Color(103, 236, 184))
            : sf::Color(164, 132, 74, 210));
        card.setOutlineThickness(highlighted ? 3.f : 1.5f);
        target.draw(card);

        drawText("[" + std::to_string(index + 1) + "] " + preview.name,
                 {cardX + 16.f, cardY + 18.f}, 17, gold, true);
        drawText(preview.effectLine1, {cardX + 16.f, cardY + 61.f}, 14, body);
        drawText(preview.effectLine2, {cardX + 16.f, cardY + 86.f}, 14, body);
        const std::string levelLine = preview.maxLevel
            ? "Level " + std::to_string(preview.level) + " (MAX)"
            : "Level " + std::to_string(preview.level) + "  ->  " +
              std::to_string(preview.nextLevel);
        drawText(levelLine, {cardX + 16.f, cardY + 135.f}, 16, teal, true);
        drawText("Cost: " + std::to_string(preview.cost) + " Gold",
                 {cardX + 16.f, cardY + 173.f}, 16, body, true);
        const std::string state = preview.maxLevel
            ? "MAX LEVEL" : (preview.affordable ? "AVAILABLE"
                                                  : "NOT ENOUGH GOLD");
        const sf::Color stateColor = preview.maxLevel
            ? muted : (preview.affordable ? sf::Color(111, 235, 174)
                                          : sf::Color(241, 116, 82));
        drawText(state, {cardX + 16.f, cardY + 214.f}, 14, stateColor, true);
    }

    const float statusY = panelPosition.y + 402.f;
    if (controller.getPromptState() ==
        IntermissionPromptState::AwaitingSkipConfirmation) {
        drawText("No upgrade selected.", {panelPosition.x + 34.f, statusY},
                 17, sf::Color(244, 151, 87), true);
        drawText("Press ENTER again to continue without upgrading.",
                 {panelPosition.x + 34.f, statusY + 25.f}, 15, body);
    } else if (controller.getFeedback() != IntermissionFeedback::None) {
        const bool error =
            controller.getFeedback() == IntermissionFeedback::NotEnoughGold ||
            controller.getFeedback() == IntermissionFeedback::MaxLevel ||
            controller.getFeedback() == IntermissionFeedback::NothingToUndo;
        drawText(std::string(intermissionFeedbackText(controller.getFeedback())),
                 {panelPosition.x + 34.f, statusY + 10.f}, 17,
                 error ? sf::Color(241, 116, 82)
                       : sf::Color(108, 236, 181), true);
    }

    const float footerY = panelPosition.y + panelHeight - 72.f;
    drawText("1 / 2 / 3", {panelPosition.x + 34.f, footerY}, 14, teal, true);
    drawText("Purchase Upgrade", {panelPosition.x + 135.f, footerY}, 14, body);
    drawText("BACKSPACE", {panelPosition.x + 345.f, footerY}, 14, teal, true);
    drawText("Undo Last Upgrade", {panelPosition.x + 455.f, footerY}, 14, body);
    drawText("ENTER", {panelPosition.x + 670.f, footerY}, 14, teal, true);
    drawText("Begin Next Wave", {panelPosition.x + 738.f, footerY}, 14, body);
    drawText("H  Show Guide", {panelPosition.x + 34.f, footerY + 28.f},
             13, muted);
}
