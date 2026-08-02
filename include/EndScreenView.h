#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

class TextureManager;

enum class EndScreenTheme {
    Victory,
    Defeat
};

class EndScreenView {
public:
    explicit EndScreenView(TextureManager& textureManager);

    void initialize(EndScreenTheme newTheme);
    void update(float deltaTime, const sf::RenderWindow& window);
    void draw(sf::RenderWindow& window) const;

    bool restartHit(sf::Vector2f position) const;
    bool menuHit(sf::Vector2f position) const;
    EndScreenTheme getTheme() const { return theme; }
    std::size_t particleCount() const { return particles.size(); }

private:
    struct AmbientParticle {
        sf::Vector2f position;
        sf::Vector2f velocity;
        sf::Color color;
        float size = 3.f;
        float rotation = 0.f;
        float spin = 0.f;
        float phase = 0.f;
    };

    TextureManager& textureManager;
    EndScreenTheme theme = EndScreenTheme::Victory;
    sf::Font font;
    bool fontLoaded = false;
    float elapsed = 0.f;

    sf::RectangleShape background;
    sf::RectangleShape tint;
    sf::RectangleShape vignette;
    sf::RectangleShape panelShadow;
    sf::RectangleShape panel;
    sf::RectangleShape restartButton;
    sf::RectangleShape menuButton;
    sf::RectangleShape leftRule;
    sf::RectangleShape rightRule;

    std::unique_ptr<sf::Text> title;
    std::unique_ptr<sf::Text> subtitle;
    std::unique_ptr<sf::Text> summary;
    std::unique_ptr<sf::Text> restartLabel;
    std::unique_ptr<sf::Text> menuLabel;
    std::unique_ptr<sf::Text> controlsHint;

    std::vector<AmbientParticle> particles;

    sf::Color accentColor() const;
    sf::Color secondaryColor() const;
    void buildText();
    void buildParticles();
    static void centerText(sf::Text& text, sf::Vector2f position);
};
