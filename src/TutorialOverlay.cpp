#include "TutorialOverlay.h"

#include "AssetLocator.h"

#include <algorithm>

void TutorialController::resetForNewGame() {
    mode = TutorialMode::FirstLaunch;
    enterHeld = false;
    helpHeld = false;
    awaitingInputRelease = false;
}

TutorialTransition TutorialController::handleKeyPressed(TutorialKey key) {
    if (awaitingInputRelease) return TutorialTransition::Consumed;

    if (key == TutorialKey::Enter) {
        if (enterHeld) return TutorialTransition::Consumed;
        enterHeld = true;
        if (mode == TutorialMode::Hidden) {
            return TutorialTransition::Ignored;
        }

        const bool initial = mode == TutorialMode::FirstLaunch;
        mode = TutorialMode::Hidden;
        awaitingInputRelease = true;
        return initial ? TutorialTransition::InitialClosed
                       : TutorialTransition::Closed;
    }

    if (helpHeld) return TutorialTransition::Consumed;
    helpHeld = true;
    if (mode == TutorialMode::FirstLaunch) {
        return TutorialTransition::Consumed;
    }
    if (mode == TutorialMode::Reopened) {
        mode = TutorialMode::Hidden;
        awaitingInputRelease = true;
        return TutorialTransition::Closed;
    }

    mode = TutorialMode::Reopened;
    return TutorialTransition::Reopened;
}

bool TutorialController::handleKeyReleased(TutorialKey key) {
    bool& held = key == TutorialKey::Enter ? enterHeld : helpHeld;
    const bool consumed = held;
    held = false;
    return consumed;
}

bool TutorialOverlay::load() {
    const auto path = AssetLocator::find("assets/fonts/Font.ttf");
    fontLoaded = path && font.openFromFile(*path);
    return fontLoaded;
}

void TutorialOverlay::draw(sf::RenderWindow& target,
                           TutorialMode mode) const {
    if (mode == TutorialMode::Hidden) return;

    const sf::View view = target.getView();
    const sf::Vector2f viewSize = view.getSize();
    const sf::Vector2f topLeft = view.getCenter() - viewSize / 2.f;

    sf::RectangleShape veil(viewSize);
    veil.setPosition(topLeft);
    veil.setFillColor(sf::Color(3, 8, 16, 205));
    target.draw(veil);

    const float panelWidth = std::clamp(viewSize.x * 0.58f, 640.f, 780.f);
    const float panelHeight = std::clamp(viewSize.y * 0.80f, 520.f, 620.f);
    const sf::Vector2f panelPosition{
        view.getCenter().x - panelWidth / 2.f,
        view.getCenter().y - panelHeight / 2.f};

    sf::RectangleShape panel({panelWidth, panelHeight});
    panel.setPosition(panelPosition);
    panel.setFillColor(sf::Color(9, 24, 34, 245));
    panel.setOutlineColor(sf::Color(210, 166, 78, 230));
    panel.setOutlineThickness(3.f);
    target.draw(panel);

    sf::RectangleShape headerLine({panelWidth - 64.f, 2.f});
    headerLine.setPosition({panelPosition.x + 32.f, panelPosition.y + 66.f});
    headerLine.setFillColor(sf::Color(65, 211, 190, 180));
    target.draw(headerLine);

    if (!fontLoaded) return;

    const auto drawText = [&](const std::string& value, sf::Vector2f position,
                              unsigned size, sf::Color color,
                              bool bold = false) {
        sf::Text text(font, value, size);
        text.setPosition(position);
        text.setFillColor(color);
        text.setOutlineColor(sf::Color(0, 0, 0, 150));
        text.setOutlineThickness(1.f);
        if (bold) text.setStyle(sf::Text::Bold);
        target.draw(text);
    };

    const sf::Color gold(242, 202, 105);
    const sf::Color teal(99, 231, 207);
    const sf::Color body(224, 232, 232);
    const sf::Color muted(163, 182, 188);
    drawText("HOW TO PLAY", {panelPosition.x + 32.f, panelPosition.y + 20.f},
             29, gold, true);
    drawText("Defend your team through 4 waves\nand defeat the final Boss.",
             {panelPosition.x + 36.f, panelPosition.y + 82.f}, 17, body);

    static constexpr const char* keys[] = {
        "WASD", "Mouse", "Left Click", "Q", "P", "M", "H"};
    static constexpr const char* actions[] = {
        "Move", "Aim", "Fire Spirit Bolt",
        "Radiant Pulse - Heal nearby Allies", "Pause",
        "Toggle Audio", "Show This Guide"};
    const float rowStart = panelPosition.y + 150.f;
    const float actionX = panelPosition.x + panelWidth * 0.34f;
    for (std::size_t index = 0; index < std::size(keys); ++index) {
        const float y = rowStart + static_cast<float>(index) * 29.f;
        drawText(keys[index], {panelPosition.x + 48.f, y}, 16, teal, true);
        drawText(actions[index], {actionX, y}, 16, body);
    }

    const float infoY = rowStart + 224.f;
    drawText("Your Allies fight automatically.",
             {panelPosition.x + 36.f, infoY}, 16, body, true);
    drawText("Survive, earn Gold and upgrade between waves.",
             {panelPosition.x + 36.f, infoY + 28.f}, 15, muted);

    const std::string footer = mode == TutorialMode::FirstLaunch
        ? "Press ENTER to begin"
        : "Press H or ENTER to resume";
    sf::Text footerText(font, footer, 20);
    footerText.setStyle(sf::Text::Bold);
    footerText.setFillColor(gold);
    footerText.setOutlineColor(sf::Color(0, 0, 0, 170));
    footerText.setOutlineThickness(1.f);
    const sf::FloatRect footerBounds = footerText.getLocalBounds();
    footerText.setOrigin({footerBounds.position.x + footerBounds.size.x / 2.f,
                          footerBounds.position.y + footerBounds.size.y / 2.f});
    footerText.setPosition({view.getCenter().x,
                            panelPosition.y + panelHeight - 38.f});
    target.draw(footerText);
}
