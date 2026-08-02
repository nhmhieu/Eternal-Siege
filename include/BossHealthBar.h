#pragma once

#include <SFML/Graphics.hpp>

#include <optional>

class Boss;

class BossHealthBar {
public:
    BossHealthBar();

    void load();
    void update(const Boss* boss, float deltaTime, bool paused);
    void draw(sf::RenderWindow& window) const;
    static sf::Vector2f calculateBarSize(sf::Vector2f viewSize);
    static float getEnrageMarkerRatio();

    bool isVisible() const { return visible; }
    float getDisplayedHealth() const { return health; }
    float getDisplayedMaxHealth() const { return maxHealth; }
    int getDisplayedPhase() const { return phase; }
    float getFlashRemaining() const { return flashRemaining; }
    bool isDisplayedEnraged() const { return enraged; }

private:
    sf::Font font;
    bool fontLoaded = false;
    bool visible = false;
    float health = 0.f;
    float maxHealth = 0.f;
    int phase = 1;
    int previousPhase = 1;
    bool enraged = false;
    float flashRemaining = 0.f;
    sf::RectangleShape backdrop;
    sf::RectangleShape healthBackground;
    sf::RectangleShape healthFill;
    std::optional<sf::Text> nameText;
    std::optional<sf::Text> valueText;
    std::optional<sf::Text> phaseText;
    std::optional<sf::Text> enrageMarkerText;
};
