#include "EndScreenView.h"

#include "AssetLocator.h"
#include "TextureManager.h"

#include <cstdint>
#include <cmath>
#include <iostream>

namespace {
constexpr sf::Vector2f screenSize{1280.f, 720.f};
}

EndScreenView::EndScreenView(TextureManager& manager)
    : textureManager(manager) {
    background.setSize(screenSize);
    tint.setSize(screenSize);
    vignette.setSize(screenSize - sf::Vector2f(28.f, 28.f));
    vignette.setPosition({14.f, 14.f});
    vignette.setFillColor(sf::Color::Transparent);
    vignette.setOutlineThickness(14.f);

    panelShadow.setSize({650.f, 466.f});
    panelShadow.setOrigin(panelShadow.getSize() / 2.f);
    panelShadow.setPosition({647.f, 389.f});
    panelShadow.setFillColor(sf::Color(0, 0, 0, 115));

    panel.setSize({630.f, 450.f});
    panel.setOrigin(panel.getSize() / 2.f);
    panel.setPosition({640.f, 380.f});
    panel.setFillColor(sf::Color(11, 14, 28, 224));
    panel.setOutlineThickness(2.f);

    restartButton.setSize({250.f, 62.f});
    restartButton.setOrigin(restartButton.getSize() / 2.f);
    restartButton.setPosition({640.f, 440.f});
    restartButton.setOutlineThickness(2.f);

    menuButton.setSize({250.f, 58.f});
    menuButton.setOrigin(menuButton.getSize() / 2.f);
    menuButton.setPosition({640.f, 520.f});
    menuButton.setOutlineThickness(2.f);

    leftRule.setSize({145.f, 2.f});
    leftRule.setOrigin({145.f, 1.f});
    leftRule.setPosition({555.f, 174.f});
    rightRule.setSize({145.f, 2.f});
    rightRule.setOrigin({0.f, 1.f});
    rightRule.setPosition({725.f, 174.f});
}

sf::Color EndScreenView::accentColor() const {
    return theme == EndScreenTheme::Victory
        ? sf::Color(255, 213, 92)
        : sf::Color(255, 74, 91);
}

sf::Color EndScreenView::secondaryColor() const {
    return theme == EndScreenTheme::Victory
        ? sf::Color(74, 224, 190)
        : sf::Color(180, 73, 255);
}

void EndScreenView::centerText(sf::Text& text, sf::Vector2f position) {
    const sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin({bounds.position.x + bounds.size.x / 2.f,
                    bounds.position.y + bounds.size.y / 2.f});
    text.setPosition(position);
}

void EndScreenView::initialize(EndScreenTheme newTheme) {
    theme = newTheme;
    elapsed = 0.f;

    if (!textureManager.findTexture("EndScreenBackground")) {
        textureManager.loadTexture(
            "EndScreenBackground", "assets/images/ui/menu_background.png");
    }
    const sf::Texture* texture =
        textureManager.findTexture("EndScreenBackground");
    if (texture && texture->getSize().x > 0) {
        background.setTexture(texture, true);
        background.setFillColor(sf::Color::White);
    } else {
        background.setFillColor(theme == EndScreenTheme::Victory
            ? sf::Color(7, 34, 37)
            : sf::Color(31, 5, 15));
    }

    tint.setFillColor(theme == EndScreenTheme::Victory
        ? sf::Color(1, 36, 42, 154)
        : sf::Color(39, 0, 17, 178));
    vignette.setOutlineColor(theme == EndScreenTheme::Victory
        ? sf::Color(8, 16, 24, 205)
        : sf::Color(18, 2, 10, 220));
    panel.setOutlineColor(sf::Color(
        accentColor().r, accentColor().g, accentColor().b, 190));
    leftRule.setFillColor(accentColor());
    rightRule.setFillColor(accentColor());

    const auto fontPath = AssetLocator::find("assets/fonts/Font.ttf");
    fontLoaded = fontPath && font.openFromFile(*fontPath);
    if (!fontLoaded) {
        std::cerr << "Failed to load font in end screen!\n";
    }
    buildText();
    buildParticles();
}

void EndScreenView::buildText() {
    title.reset();
    subtitle.reset();
    summary.reset();
    restartLabel.reset();
    menuLabel.reset();
    controlsHint.reset();
    if (!fontLoaded) return;

    const bool victory = theme == EndScreenTheme::Victory;
    title = std::make_unique<sf::Text>(
        font, victory ? "VICTORY" : "DEFEAT", 72);
    title->setStyle(sf::Text::Bold);
    title->setLetterSpacing(1.18f);
    title->setFillColor(accentColor());
    centerText(*title, {640.f, 214.f});

    subtitle = std::make_unique<sf::Text>(
        font,
        victory ? "THE ETERNAL SIEGE IS BROKEN"
                : "THE FORTRESS HAS FALLEN",
        25);
    subtitle->setLetterSpacing(1.08f);
    subtitle->setFillColor(sf::Color(238, 242, 255));
    centerText(*subtitle, {640.f, 284.f});

    summary = std::make_unique<sf::Text>(
        font,
        victory ? "WAVE 4 CLEARED  |  THE REALM ENDURES"
                : "THE ENEMY BROKE THROUGH  |  RALLY ONCE MORE",
        16);
    summary->setFillColor(sf::Color(174, 185, 210));
    centerText(*summary, {640.f, 330.f});

    restartLabel = std::make_unique<sf::Text>(font, "PLAY AGAIN", 24);
    restartLabel->setStyle(sf::Text::Bold);
    restartLabel->setFillColor(sf::Color(14, 18, 29));
    centerText(*restartLabel, restartButton.getPosition());

    menuLabel = std::make_unique<sf::Text>(font, "MAIN MENU", 22);
    menuLabel->setFillColor(sf::Color(232, 237, 250));
    centerText(*menuLabel, menuButton.getPosition());

    controlsHint = std::make_unique<sf::Text>(
        font, "ENTER: PLAY AGAIN   K: KINGDOM   ESC: MAIN MENU", 14);
    controlsHint->setFillColor(sf::Color(137, 149, 176));
    centerText(*controlsHint, {640.f, 588.f});
}

void EndScreenView::buildParticles() {
    particles.clear();
    const bool victory = theme == EndScreenTheme::Victory;
    const sf::Color colors[] = {
        accentColor(), secondaryColor(), sf::Color(245, 245, 255),
        victory ? sf::Color(94, 188, 255) : sf::Color(255, 126, 72)
    };

    for (int i = 0; i < 46; ++i) {
        AmbientParticle particle;
        particle.position = {
            static_cast<float>((i * 83 + 37) % 1280),
            static_cast<float>((i * 137 + 19) % 720)};
        particle.velocity = victory
            ? sf::Vector2f(static_cast<float>((i % 5) - 2) * 4.f,
                           25.f + static_cast<float>(i % 7) * 7.f)
            : sf::Vector2f(static_cast<float>((i % 7) - 3) * 3.f,
                           -18.f - static_cast<float>(i % 9) * 6.f);
        particle.color = colors[i % 4];
        particle.color.a = static_cast<std::uint8_t>(90 + (i % 5) * 24);
        particle.size = victory ? 2.5f + static_cast<float>(i % 5)
                                : 1.5f + static_cast<float>(i % 4);
        particle.rotation = static_cast<float>((i * 31) % 180);
        particle.spin = static_cast<float>((i % 2 == 0 ? 1 : -1) *
                                           (30 + (i % 6) * 18));
        particle.phase = static_cast<float>(i) * 0.47f;
        particles.push_back(particle);
    }
}

void EndScreenView::update(float deltaTime, const sf::RenderWindow& window) {
    elapsed += deltaTime;
    const bool victory = theme == EndScreenTheme::Victory;
    for (auto& particle : particles) {
        particle.position += particle.velocity * deltaTime;
        particle.position.x += std::sin(elapsed * 1.7f + particle.phase) *
                               8.f * deltaTime;
        particle.rotation += particle.spin * deltaTime;
        if (victory && particle.position.y > 735.f) {
            particle.position.y = -15.f;
        } else if (!victory && particle.position.y < -18.f) {
            particle.position.y = 738.f;
        }
        if (particle.position.x < -20.f) particle.position.x = 1300.f;
        if (particle.position.x > 1300.f) particle.position.x = -20.f;
    }

    const sf::Vector2f mouse = window.mapPixelToCoords(
        sf::Mouse::getPosition(window));
    const bool restartHover = restartHit(mouse);
    const bool menuHover = menuHit(mouse);
    const sf::Color accent = accentColor();

    restartButton.setFillColor(restartHover
        ? sf::Color(255, 238, 170)
        : accent);
    restartButton.setOutlineColor(restartHover
        ? sf::Color::White
        : sf::Color(accent.r, accent.g, accent.b, 145));
    restartButton.setScale(restartHover
        ? sf::Vector2f(1.035f, 1.035f)
        : sf::Vector2f(1.f, 1.f));

    menuButton.setFillColor(menuHover
        ? sf::Color(54, 62, 86, 245)
        : sf::Color(27, 32, 50, 235));
    menuButton.setOutlineColor(menuHover
        ? secondaryColor()
        : sf::Color(113, 124, 153, 160));
    menuButton.setScale(menuHover
        ? sf::Vector2f(1.025f, 1.025f)
        : sf::Vector2f(1.f, 1.f));

    if (title) {
        const float pulse = 1.f + std::sin(elapsed * 2.8f) * 0.018f;
        title->setScale({pulse, pulse});
    }
}

bool EndScreenView::restartHit(sf::Vector2f position) const {
    return restartButton.getGlobalBounds().contains(position);
}

bool EndScreenView::menuHit(sf::Vector2f position) const {
    return menuButton.getGlobalBounds().contains(position);
}

void EndScreenView::draw(sf::RenderWindow& window) const {
    window.draw(background);
    window.draw(tint);

    for (const auto& particle : particles) {
        sf::RectangleShape shape({particle.size, particle.size *
            (theme == EndScreenTheme::Victory ? 2.2f : 1.f)});
        shape.setOrigin(shape.getSize() / 2.f);
        shape.setPosition(particle.position);
        shape.setRotation(sf::degrees(particle.rotation));
        sf::Color color = particle.color;
        color.a = static_cast<std::uint8_t>(
            static_cast<float>(color.a) *
            (0.72f + std::sin(elapsed * 2.f + particle.phase) * 0.28f));
        shape.setFillColor(color);
        window.draw(shape);
    }

    window.draw(panelShadow);
    window.draw(panel);

    const sf::Color accent = accentColor();
    const float pulse = 1.f + std::sin(elapsed * 2.8f) * 0.07f;
    sf::CircleShape outer(64.f);
    outer.setOrigin({64.f, 64.f});
    outer.setPosition({640.f, 174.f});
    outer.setScale({pulse, pulse});
    outer.setFillColor(sf::Color(accent.r, accent.g, accent.b, 18));
    outer.setOutlineColor(sf::Color(accent.r, accent.g, accent.b, 95));
    outer.setOutlineThickness(2.f);
    window.draw(outer);

    sf::CircleShape inner(48.f);
    inner.setOrigin({48.f, 48.f});
    inner.setPosition({640.f, 174.f});
    inner.setFillColor(sf::Color(9, 12, 25, 215));
    inner.setOutlineColor(accent);
    inner.setOutlineThickness(3.f);
    window.draw(inner);
    window.draw(leftRule);
    window.draw(rightRule);

    window.draw(restartButton);
    window.draw(menuButton);
    if (title) window.draw(*title);
    if (subtitle) window.draw(*subtitle);
    if (summary) window.draw(*summary);
    if (restartLabel) window.draw(*restartLabel);
    if (menuLabel) window.draw(*menuLabel);
    if (controlsHint) window.draw(*controlsHint);
    window.draw(vignette);
}
