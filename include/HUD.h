#pragma once

#include <SFML/Graphics.hpp>

#include <array>
#include <memory>
#include <string>
#include <vector>

class Ally;
class Player;
class RadiantPulse;
class UpgradeManager;
class WaveManager;

struct HUDLayout {
    sf::FloatRect playerCard;
    sf::FloatRect allyCard;
    sf::FloatRect upgradeCard;
    sf::FloatRect radiantCard;
    sf::FloatRect controlsCard;
};

class HUD {
public:
    HUD() = default;

    void load();
    void update(const Player& player, const WaveManager& wave,
                const UpgradeManager& upgrades, int enemiesAlive,
                const RadiantPulse& radiantPulse,
                const std::vector<std::unique_ptr<Ally>>& allies);
    void updateAnimation(float deltaTime, bool paused);
    void notifyRadiantCast();
    void setSoundMuted(bool value) { soundMuted = value; }
    void draw(sf::RenderWindow& window) const;

    static HUDLayout calculateLayout(sf::Vector2f viewSize);

private:
    bool soundMuted = false;
    struct AllySnapshot {
        std::string name;
        float health = 0.f;
        float maxHealth = 1.f;
        bool dead = false;
        sf::Color color = sf::Color::White;
    };

    sf::Font font;
    bool fontLoaded = false;
    float playerHealth = 0.f;
    float playerMaxHealth = 1.f;
    float playerDamage = 0.f;
    float playerCooldown = 0.f;
    int gold = 0;
    int waveNumber = 0;
    int maxWaves = 0;
    int enemies = 0;
    bool waveActive = false;
    bool intermission = false;
    bool radiantReady = true;
    float radiantCooldown = 0.f;
    float radiantFlash = 0.f;
    std::array<int, 3> upgradeLevels{};
    std::array<int, 3> upgradeCosts{};
    std::vector<AllySnapshot> allySnapshots;

    void drawCard(sf::RenderWindow& window, const sf::FloatRect& bounds,
                  sf::Color outline, sf::Color fill) const;
    void drawLabel(sf::RenderWindow& window, const std::string& value,
                   sf::Vector2f position, unsigned size, sf::Color color,
                   bool bold = false) const;
    void drawHealthBar(sf::RenderWindow& window, sf::Vector2f position,
                       sf::Vector2f size, float health, float maxHealth,
                       sf::Color fill) const;
};
