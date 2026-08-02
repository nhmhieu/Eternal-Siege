#include "HUD.h"

#include "Ally.h"
#include "AssetLocator.h"
#include "Player.h"
#include "RadiantPulse.h"
#include "UpgradeManager.h"
#include "WaveManager.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace {
const sf::Color PANEL(10, 19, 29, 226);
const sf::Color CARD(18, 31, 43, 236);
const sf::Color BRONZE(205, 154, 77, 205);
const sf::Color TEAL(76, 225, 198);
const sf::Color TEXT(232, 239, 242);
const sf::Color MUTED(143, 159, 170);

sf::Color allyColor(AllyType type) {
    switch (type) {
    case AllyType::Junior: return sf::Color(187, 105, 245);
    case AllyType::Damian: return sf::Color(238, 192, 84);
    case AllyType::Evangeline: return sf::Color(105, 205, 245);
    case AllyType::Lucas: return sf::Color(235, 105, 63);
    }
    return sf::Color::White;
}
}

HUDLayout HUD::calculateLayout(sf::Vector2f viewSize) {
    viewSize.x = std::max(viewSize.x, 640.f);
    viewSize.y = std::max(viewSize.y, 480.f);
    const float margin = std::clamp(viewSize.x * 0.0125f, 10.f, 18.f);
    const float hudStart = std::clamp(viewSize.x * 0.56f, 0.f,
                                      viewSize.x - 330.f);
    const float available = std::max(310.f, viewSize.x - hudStart - margin);
    const float gap = 10.f;
    const float leftWidth = std::clamp(available * 0.47f, 230.f, 280.f);
    const float rightWidth = std::max(70.f, available - leftWidth - gap);
    const float top = margin;
    const float left = hudStart;
    const float right = left + leftWidth + gap;

    HUDLayout result;
    result.playerCard = {{left, top}, {leftWidth, 92.f}};
    result.allyCard = {{left, top + 102.f}, {leftWidth, 190.f}};
    result.upgradeCard = {{right, top}, {rightWidth, 238.f}};
    result.radiantCard = {{left, viewSize.y - 86.f}, {leftWidth, 68.f}};
    result.controlsCard = {{right, viewSize.y - 70.f}, {rightWidth, 52.f}};
    return result;
}

void HUD::load() {
    const auto path = AssetLocator::find("assets/fonts/Font.ttf");
    fontLoaded = path && font.openFromFile(*path);
}

void HUD::update(const Player& player, const WaveManager& wave,
                 const UpgradeManager& upgrades, int enemiesAlive,
                 const RadiantPulse& radiantPulse,
                 const std::vector<std::unique_ptr<Ally>>& allies) {
    playerHealth = player.getHealth();
    playerMaxHealth = player.getMaxHealth();
    playerDamage = player.getAttackPower();
    playerCooldown = player.getAttackCooldown();
    gold = upgrades.getGold();
    waveNumber = wave.getCurrentWave();
    maxWaves = wave.getMaxWaves();
    enemies = enemiesAlive;
    waveActive = wave.isWaveActive();
    intermission = wave.isIntermission();
    radiantReady = radiantPulse.isReady();
    radiantCooldown = radiantPulse.getCooldownRemaining();
    for (int index = 0; index < 3; ++index) {
        const auto type = static_cast<UpgradeType>(index);
        upgradeLevels[static_cast<std::size_t>(index)] = upgrades.getLevel(type);
        upgradeCosts[static_cast<std::size_t>(index)] = upgrades.getCost(type);
    }

    allySnapshots.clear();
    allySnapshots.reserve(4);
    for (const AllyType type : {AllyType::Damian, AllyType::Evangeline,
                                AllyType::Junior, AllyType::Lucas}) {
        allySnapshots.push_back({getAllyName(type), 0.f,
            getAllyStats(type).maxHealth, true, allyColor(type)});
    }
    for (const auto& ally : allies) {
        if (!ally) continue;
        const std::size_t index = static_cast<std::size_t>(ally->getType());
        if (index < allySnapshots.size()) {
            allySnapshots[index] = {getAllyName(ally->getType()),
                ally->getHealth(), ally->getMaxHealth(), ally->isDead(),
                allyColor(ally->getType())};
        }
    }
}

void HUD::updateAnimation(float deltaTime, bool paused) {
    if (!paused) radiantFlash = std::max(0.f, radiantFlash - deltaTime);
}

void HUD::notifyRadiantCast() {
    radiantFlash = 0.35f;
}

void HUD::drawCard(sf::RenderWindow& window, const sf::FloatRect& bounds,
                   sf::Color outline, sf::Color fill) const {
    sf::RectangleShape shadow(bounds.size + sf::Vector2f(4.f, 4.f));
    shadow.setPosition(bounds.position + sf::Vector2f(3.f, 4.f));
    shadow.setFillColor(sf::Color(0, 0, 0, 90));
    window.draw(shadow);
    sf::RectangleShape card(bounds.size);
    card.setPosition(bounds.position);
    card.setFillColor(fill);
    card.setOutlineColor(outline);
    card.setOutlineThickness(1.5f);
    window.draw(card);
}

void HUD::drawLabel(sf::RenderWindow& window, const std::string& value,
                    sf::Vector2f position, unsigned size, sf::Color color,
                    bool bold) const {
    if (!fontLoaded) return;
    sf::Text shadow(font, value, size);
    if (bold) shadow.setStyle(sf::Text::Bold);
    shadow.setPosition(position + sf::Vector2f(1.5f, 1.5f));
    shadow.setFillColor(sf::Color(0, 0, 0, color.a));
    window.draw(shadow);
    sf::Text text(font, value, size);
    if (bold) text.setStyle(sf::Text::Bold);
    text.setPosition(position);
    text.setFillColor(color);
    window.draw(text);
}

void HUD::drawHealthBar(sf::RenderWindow& window, sf::Vector2f position,
                        sf::Vector2f size, float health, float maxHealth,
                        sf::Color fill) const {
    sf::RectangleShape background(size);
    background.setPosition(position);
    background.setFillColor(sf::Color(7, 12, 18, 235));
    background.setOutlineColor(sf::Color(110, 120, 125, 170));
    background.setOutlineThickness(1.f);
    window.draw(background);
    const float ratio = maxHealth > 0.f
        ? std::clamp(health / maxHealth, 0.f, 1.f) : 0.f;
    sf::RectangleShape bar({std::max(0.f, size.x * ratio), size.y});
    bar.setPosition(position);
    bar.setFillColor(fill);
    window.draw(bar);
}

void HUD::draw(sf::RenderWindow& window) const {
    const sf::View& view = window.getView();
    const sf::Vector2f size = view.getSize();
    const sf::Vector2f topLeft = view.getCenter() - size / 2.f;
    HUDLayout layout = calculateLayout(size);
    auto translate = [topLeft](sf::FloatRect& rect) { rect.position += topLeft; };
    translate(layout.playerCard);
    translate(layout.allyCard);
    translate(layout.upgradeCard);
    translate(layout.radiantCard);
    translate(layout.controlsCard);

    const sf::FloatRect panelBounds{
        {layout.playerCard.position.x - 10.f, topLeft.y},
        {size.x - (layout.playerCard.position.x - topLeft.x) + 10.f, size.y}};
    sf::RectangleShape panel(panelBounds.size);
    panel.setPosition(panelBounds.position);
    panel.setFillColor(PANEL);
    window.draw(panel);

    drawCard(window, layout.playerCard, BRONZE, CARD);
    drawLabel(window, "PLAYER MAGE  /  SPIRIT WARDEN",
              layout.playerCard.position + sf::Vector2f(12.f, 9.f),
              14, sf::Color(235, 205, 135), true);
    drawHealthBar(window, layout.playerCard.position + sf::Vector2f(12.f, 35.f),
                  {layout.playerCard.size.x - 24.f, 15.f},
                  playerHealth, playerMaxHealth, TEAL);
    std::ostringstream playerLine;
    playerLine << static_cast<int>(playerHealth) << "/"
               << static_cast<int>(playerMaxHealth) << " HP   DMG "
               << static_cast<int>(playerDamage) << "   "
               << std::fixed << std::setprecision(2) << playerCooldown << "s";
    drawLabel(window, playerLine.str(),
              layout.playerCard.position + sf::Vector2f(12.f, 57.f),
              12, TEXT);

    drawCard(window, layout.allyCard, sf::Color(119, 132, 141, 180), CARD);
    drawLabel(window, "GUARDIANS",
              layout.allyCard.position + sf::Vector2f(12.f, 8.f),
              13, MUTED, true);
    for (std::size_t index = 0; index < allySnapshots.size() && index < 4; ++index) {
        const auto& ally = allySnapshots[index];
        const float y = layout.allyCard.position.y + 36.f +
                        static_cast<float>(index) * 37.f;
        drawLabel(window, ally.name, {layout.allyCard.position.x + 12.f, y},
                  12, ally.dead ? MUTED : ally.color, true);
        drawHealthBar(window, {layout.allyCard.position.x + 93.f, y + 2.f},
            {layout.allyCard.size.x - 105.f, 10.f}, ally.health,
            ally.maxHealth, ally.dead ? sf::Color(75, 75, 80) : ally.color);
    }

    drawCard(window, layout.upgradeCard, BRONZE, CARD);
    drawLabel(window, "WAR CHEST",
              layout.upgradeCard.position + sf::Vector2f(13.f, 9.f),
              14, sf::Color(235, 205, 135), true);
    drawLabel(window, "GOLD  " + std::to_string(gold),
              layout.upgradeCard.position + sf::Vector2f(13.f, 34.f),
              18, sf::Color(244, 194, 83), true);
    drawLabel(window, "WAVE " + std::to_string(waveNumber) + "/" +
              std::to_string(maxWaves) + "   ENEMIES " +
              std::to_string(enemies),
              layout.upgradeCard.position + sf::Vector2f(13.f, 62.f),
              12, TEXT);
    const char* labels[] = {"[1] DAMAGE +4", "[2] VITALITY +25",
                            "[3] FIRE RATE +16%"};
    for (int index = 0; index < 3; ++index) {
        const float y = layout.upgradeCard.position.y + 92.f + index * 38.f;
        drawLabel(window, labels[index],
                  {layout.upgradeCard.position.x + 13.f, y}, 12,
                  intermission ? TEXT : MUTED, true);
        drawLabel(window, "Lv." + std::to_string(upgradeLevels[index]) +
                  "  " + std::to_string(upgradeCosts[index]) + "g",
                  {layout.upgradeCard.position.x + 13.f, y + 17.f}, 11, MUTED);
    }
    if (intermission) {
        drawLabel(window, "BACKSPACE  UNDO    ENTER  NEXT WAVE",
                  layout.upgradeCard.position + sf::Vector2f(13.f, 211.f),
                  10, sf::Color(123, 220, 198));
    }

    const sf::Color radiantOutline = radiantFlash > 0.f
        ? sf::Color(255, 231, 145) : sf::Color(70, 185, 165, 205);
    drawCard(window, layout.radiantCard, radiantOutline, CARD);
    drawLabel(window, "[Q]  RADIANT PULSE",
              layout.radiantCard.position + sf::Vector2f(12.f, 8.f),
              13, waveActive ? TEXT : MUTED, true);
    std::string status = "INACTIVE";
    sf::Color statusColor = MUTED;
    if (waveActive && radiantReady) {
        status = "READY";
        statusColor = sf::Color(112, 238, 191);
    } else if (waveActive) {
        std::ostringstream cooldown;
        cooldown << std::fixed << std::setprecision(1) << radiantCooldown << "s";
        status = cooldown.str();
        statusColor = sf::Color(238, 194, 91);
        const float ratio = std::clamp(radiantCooldown / RadiantPulse::COOLDOWN,
                                       0.f, 1.f);
        sf::RectangleShape overlay({layout.radiantCard.size.x * ratio,
                                    layout.radiantCard.size.y});
        overlay.setPosition(layout.radiantCard.position);
        overlay.setFillColor(sf::Color(4, 8, 13, 105));
        window.draw(overlay);
    }
    drawLabel(window, status,
              layout.radiantCard.position + sf::Vector2f(12.f, 35.f),
              15, statusColor, true);

    drawCard(window, layout.controlsCard, sf::Color(79, 94, 105, 100),
             sf::Color(14, 25, 35, 180));
    drawLabel(window, "WASD MOVE   LMB SHOOT   P PAUSE",
              layout.controlsCard.position + sf::Vector2f(10.f, 9.f),
              10, MUTED);
    drawLabel(window, soundMuted ? "M: SOUND OFF" : "M: SOUND ON",
              layout.controlsCard.position + sf::Vector2f(10.f, 27.f),
              10, soundMuted ? sf::Color(151, 132, 132)
                              : sf::Color(112, 205, 181));
}
