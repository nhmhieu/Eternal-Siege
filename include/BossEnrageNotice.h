#pragma once

#include <SFML/Graphics.hpp>

#include <optional>

class BossEnrageNotice {
public:
    static constexpr float DISPLAY_DURATION = 2.1f;

    void resetForNewGame();
    bool observe(bool bossAlive, bool bossEnraged);
    void update(float deltaTime, bool paused);

    bool load();
    void draw(sf::RenderWindow& target) const;

    bool isVisible() const { return remaining > 0.f; }
    bool hasBeenShown() const { return shown; }
    float getRemaining() const { return remaining; }
    int getActivationCount() const { return activationCount; }

private:
    sf::Font font;
    bool fontLoaded = false;
    bool previousEnraged = false;
    bool shown = false;
    float remaining = 0.f;
    int activationCount = 0;
    std::optional<sf::Text> title;
    std::optional<sf::Text> subtitle;
};
