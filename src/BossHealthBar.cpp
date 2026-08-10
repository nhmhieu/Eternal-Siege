#include "BossHealthBar.h"

#include "AssetLocator.h"
#include "Boss.h"

#include <algorithm>
#include <sstream>
#include <string>

namespace {
constexpr float BAR_X = 360.f;
constexpr float BAR_Y = 42.f;
constexpr float BAR_WIDTH = 540.f;
constexpr float BAR_HEIGHT = 19.f;

const char* romanPhase(int phase) {
    switch (phase) {
    case 2: return "PHASE II";
    case 3: return "PHASE III";
    default: return "PHASE I";
    }
}
}

sf::Vector2f BossHealthBar::calculateBarSize(sf::Vector2f viewSize) {
    return {
        std::clamp(viewSize.x * 0.42f, 360.f, 560.f),
        std::clamp(viewSize.y * 0.026f, 16.f, 21.f)};
}

float BossHealthBar::getEnrageMarkerRatio() {
    return Boss::getEnrageThresholdRatio();
}

BossHealthBar::BossHealthBar() {
    backdrop.setPosition({BAR_X - 16.f, BAR_Y - 30.f});
    backdrop.setSize({BAR_WIDTH + 32.f, 82.f});
    backdrop.setFillColor(sf::Color(12, 9, 22, 225));
    backdrop.setOutlineColor(sf::Color(160, 80, 195, 180));
    backdrop.setOutlineThickness(2.f);

    healthBackground.setPosition({BAR_X, BAR_Y});
    healthBackground.setSize({BAR_WIDTH, BAR_HEIGHT});
    healthBackground.setFillColor(sf::Color(38, 28, 48, 245));
    healthBackground.setOutlineColor(sf::Color(230, 160, 255, 180));
    healthBackground.setOutlineThickness(1.f);

    healthFill.setPosition({BAR_X, BAR_Y});
    healthFill.setSize({BAR_WIDTH, BAR_HEIGHT});
    healthFill.setFillColor(sf::Color(175, 42, 105));
}

void BossHealthBar::load() {
    const auto path = AssetLocator::find("assets/fonts/Font.ttf");
    fontLoaded = path && font.openFromFile(*path);
    if (!fontLoaded) return;
    nameText.emplace(font, "ABYSSAL LORD", 18);
    nameText->setPosition({BAR_X, BAR_Y - 27.f});
    nameText->setFillColor(sf::Color(245, 220, 255));
    valueText.emplace(font, "", 14);
    valueText->setPosition({BAR_X + BAR_WIDTH - 125.f, BAR_Y - 24.f});
    valueText->setFillColor(sf::Color(235, 225, 240));
    phaseText.emplace(font, "", 14);
    phaseText->setPosition({BAR_X + BAR_WIDTH + 10.f, BAR_Y + 2.f});
    phaseText->setFillColor(sf::Color(255, 190, 95));
    enrageMarkerText.emplace(font, "ENRAGE 25%", 9);
    enrageMarkerText->setFillColor(sf::Color(255, 195, 115));
}

void BossHealthBar::update(const Boss* boss, float deltaTime, bool paused) {
    if (paused) return;
    flashRemaining = std::max(0.f, flashRemaining - deltaTime);
    if (!boss || boss->isDead()) {
        visible = false;
        health = 0.f;
        maxHealth = 0.f;
        phase = 1;
        previousPhase = 1;
        enraged = false;
        return;
    }

    visible = true;
    health = boss->getHealth();
    maxHealth = boss->getMaxHealth();
    phase = static_cast<int>(boss->getPhase()) + 1;
    enraged = boss->isEnraged();
    if (phase != previousPhase) {
        previousPhase = phase;
        flashRemaining = 0.45f;
    }

    const float ratio = maxHealth > 0.f
        ? std::clamp(health / maxHealth, 0.f, 1.f)
        : 0.f;
    healthFill.setSize({BAR_WIDTH * ratio, BAR_HEIGHT});
    healthFill.setFillColor(phase == 3
        ? sf::Color(225, 45, 85)
        : (phase == 2 ? sf::Color(205, 65, 135)
                      : sf::Color(155, 55, 180)));
    if (flashRemaining > 0.f) {
        backdrop.setOutlineColor(sf::Color(255, 225, 115));
    } else {
        backdrop.setOutlineColor(sf::Color(160, 80, 195, 180));
    }

    if (valueText) {
        std::ostringstream value;
        value << static_cast<int>(health) << " / "
              << static_cast<int>(maxHealth);
        valueText->setString(value.str());
    }
    if (phaseText) {
        phaseText->setString(enraged ? "ENRAGED" : romanPhase(phase));
    }
}

void BossHealthBar::draw(sf::RenderWindow& window) const {
    if (!visible) return;

    const sf::View& view = window.getView();
    const sf::Vector2f viewSize = view.getSize();
    const sf::Vector2f barSize = calculateBarSize(viewSize);
    const sf::Vector2f viewTopLeft = view.getCenter() - viewSize / 2.f;
    const float x = view.getCenter().x - barSize.x / 2.f;
    const float y = viewTopLeft.y + std::max(34.f, viewSize.y * 0.047f);

    sf::RectangleShape localBackdrop({barSize.x + 22.f, barSize.y + 34.f});
    localBackdrop.setPosition({x - 11.f, y - 25.f});
    localBackdrop.setFillColor(sf::Color(12, 9, 22, 218));
    localBackdrop.setOutlineColor(flashRemaining > 0.f
        ? sf::Color(255, 225, 115, 210)
        : sf::Color(160, 80, 195, 170));
    localBackdrop.setOutlineThickness(1.5f);
    window.draw(localBackdrop);

    sf::RectangleShape localBackground(barSize);
    localBackground.setPosition({x, y});
    localBackground.setFillColor(sf::Color(38, 28, 48, 245));
    localBackground.setOutlineColor(sf::Color(225, 165, 245, 175));
    localBackground.setOutlineThickness(1.f);
    window.draw(localBackground);

    const float ratio = maxHealth > 0.f
        ? std::clamp(health / maxHealth, 0.f, 1.f) : 0.f;
    sf::RectangleShape localFill({barSize.x * ratio, barSize.y});
    localFill.setPosition({x, y});
    localFill.setFillColor(phase == 3
        ? sf::Color(225, 45, 85)
        : (phase == 2 ? sf::Color(205, 65, 135)
                      : sf::Color(155, 55, 180)));
    window.draw(localFill);

    for (float markerRatio : {
             Boss::PHASE_TWO_HEALTH_RATIO,
             Boss::getEnrageThresholdRatio()}) {
        sf::RectangleShape marker({2.f, barSize.y});
        marker.setPosition({x + barSize.x * markerRatio, y});
        marker.setFillColor(sf::Color(255, 225, 185, 210));
        window.draw(marker);
    }
    if (enrageMarkerText) {
        auto text = *enrageMarkerText;
        const sf::FloatRect bounds = text.getLocalBounds();
        const float markerX =
            x + barSize.x * Boss::getEnrageThresholdRatio();
        text.setPosition({markerX - bounds.size.x / 2.f,
                          y + barSize.y + 3.f});
        window.draw(text);
    }
    if (nameText) {
        auto text = *nameText;
        text.setCharacterSize(15);
        text.setPosition({x, y - 22.f});
        window.draw(text);
    }
    if (valueText) {
        auto text = *valueText;
        text.setCharacterSize(12);
        const sf::FloatRect bounds = text.getLocalBounds();
        text.setPosition({x + barSize.x - bounds.size.x, y - 20.f});
        window.draw(text);
    }
    if (phaseText) {
        auto text = *phaseText;
        text.setCharacterSize(12);
        const sf::FloatRect bounds = text.getLocalBounds();
        text.setPosition({x + barSize.x / 2.f - bounds.size.x / 2.f,
                          y + 1.f});
        window.draw(text);
    }
}
