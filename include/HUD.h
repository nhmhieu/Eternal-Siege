#pragma once

#include <SFML/Graphics.hpp>
#include <optional>

class Player;
class UpgradeManager;
class WaveManager;

class HUD {
private:
    sf::Font font;
    bool fontLoaded = false;

    // Mỗi nhóm dùng cỡ chữ riêng.
    std::optional<sf::Text> titleText;
    std::optional<sf::Text> statsText;
    std::optional<sf::Text> upgradeTitleText;
    std::optional<sf::Text> upgradeText;
    std::optional<sf::Text> helpText;

    // Các khối nền.
    sf::RectangleShape panel;
    sf::RectangleShape accentBar;
    sf::RectangleShape statsCard;
    sf::RectangleShape upgradesCard;
    sf::RectangleShape helpCard;

public:
    HUD();

    void load();

    void update(
        const Player& player,
        const WaveManager& wave,
        const UpgradeManager& upgrades,
        int enemiesAlive
    );

    void draw(sf::RenderWindow& window) const;
};