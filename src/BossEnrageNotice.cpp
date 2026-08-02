#include "BossEnrageNotice.h"

#include "AssetLocator.h"

#include <algorithm>
#include <cstdint>
#include <cmath>

void BossEnrageNotice::resetForNewGame() {
    previousEnraged = false;
    shown = false;
    remaining = 0.f;
    activationCount = 0;
}

bool BossEnrageNotice::observe(bool bossAlive, bool bossEnraged) {
    const bool current = bossAlive && bossEnraged;
    const bool activated = current && !previousEnraged && !shown;
    previousEnraged = current;
    if (!activated) return false;

    shown = true;
    remaining = DISPLAY_DURATION;
    ++activationCount;
    return true;
}

void BossEnrageNotice::update(float deltaTime, bool paused) {
    if (paused) return;
    remaining = std::max(0.f, remaining - std::max(0.f, deltaTime));
}

bool BossEnrageNotice::load() {
    const auto path = AssetLocator::find("assets/fonts/Font.ttf");
    fontLoaded = path && font.openFromFile(*path);
    if (!fontLoaded) return false;

    title.emplace(font, "BOSS ENRAGED", 34);
    title->setStyle(sf::Text::Bold);
    title->setFillColor(sf::Color(255, 80, 115));
    title->setOutlineColor(sf::Color(65, 5, 25, 230));
    title->setOutlineThickness(2.f);

    subtitle.emplace(
        font, "Movement Speed +45%  -  Damage Taken -35%", 16);
    subtitle->setFillColor(sf::Color(255, 215, 125));
    subtitle->setOutlineColor(sf::Color(45, 8, 20, 220));
    subtitle->setOutlineThickness(1.f);
    return true;
}

void BossEnrageNotice::draw(sf::RenderWindow& target) const {
    if (!fontLoaded || !isVisible() || !title || !subtitle) return;

    const sf::View& view = target.getView();
    const sf::Vector2f size = view.getSize();
    const sf::Vector2f topLeft = view.getCenter() - size / 2.f;
    const float elapsed = DISPLAY_DURATION - remaining;
    const float fadeIn = std::clamp(elapsed / 0.18f, 0.f, 1.f);
    const float fadeOut = std::clamp(remaining / 0.42f, 0.f, 1.f);
    const float alphaFactor = std::min(fadeIn, fadeOut);
    const auto alpha = static_cast<std::uint8_t>(220.f * alphaFactor);

    sf::RectangleShape panel({
        std::clamp(size.x * 0.39f, 430.f, 620.f), 80.f});
    panel.setOrigin({panel.getSize().x / 2.f, 0.f});
    panel.setPosition({view.getCenter().x, topLeft.y + size.y * 0.14f});
    panel.setFillColor(sf::Color(35, 4, 24, alpha));
    panel.setOutlineColor(sf::Color(255, 75, 110, alpha));
    panel.setOutlineThickness(2.f);
    target.draw(panel);

    auto localTitle = *title;
    auto localSubtitle = *subtitle;
    auto centerText = [&](sf::Text& text, float y) {
        const sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({bounds.position.x + bounds.size.x / 2.f,
                        bounds.position.y + bounds.size.y / 2.f});
        text.setPosition({view.getCenter().x, y});
        sf::Color fill = text.getFillColor();
        fill.a = static_cast<std::uint8_t>(255.f * alphaFactor);
        text.setFillColor(fill);
        sf::Color outline = text.getOutlineColor();
        outline.a = static_cast<std::uint8_t>(230.f * alphaFactor);
        text.setOutlineColor(outline);
        target.draw(text);
    };
    centerText(localTitle, panel.getPosition().y + 27.f);
    centerText(localSubtitle, panel.getPosition().y + 58.f);
}
