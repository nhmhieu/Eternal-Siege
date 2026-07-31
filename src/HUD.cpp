#include "HUD.h"

#include "Constants.h"
#include "Player.h"
#include "UpgradeManager.h"
#include "WaveManager.h"

#include <sstream>

namespace {
    const sf::Color PANEL_COLOR(14, 18, 26, 245);
    const sf::Color CARD_COLOR(24, 30, 42, 245);
    const sf::Color CARD_OUTLINE_COLOR(48, 58, 76);
    const sf::Color ACCENT_COLOR(220, 70, 75);
    const sf::Color PRIMARY_TEXT_COLOR(238, 241, 246);
    const sf::Color SECONDARY_TEXT_COLOR(170, 181, 199);
    const sf::Color GOLD_COLOR(244, 194, 83);

    constexpr float PANEL_PADDING = 24.f;
}

HUD::HUD() {
    const float panelWidth =
        GameConfig::WINDOW_WIDTH - GameConfig::HUD_LEFT;

    // Nền toàn bộ HUD.
    panel.setPosition(
        sf::Vector2f(GameConfig::HUD_LEFT, 0.f)
    );

    panel.setSize(
        sf::Vector2f(
            panelWidth,
            GameConfig::WINDOW_HEIGHT
        )
    );

    panel.setFillColor(PANEL_COLOR);

    // Thanh màu bên trái giúp tách HUD khỏi map.
    accentBar.setPosition(
        sf::Vector2f(GameConfig::HUD_LEFT, 0.f)
    );

    accentBar.setSize(
        sf::Vector2f(
            3.f,
            GameConfig::WINDOW_HEIGHT
        )
    );

    accentBar.setFillColor(ACCENT_COLOR);

    // Card chứa HP, vàng, wave...
    statsCard.setPosition(
        sf::Vector2f(
            GameConfig::HUD_LEFT + PANEL_PADDING,
            82.f
        )
    );

    statsCard.setSize(
        sf::Vector2f(
            panelWidth - PANEL_PADDING * 2.f,
            142.f
        )
    );

    statsCard.setFillColor(CARD_COLOR);
    statsCard.setOutlineColor(CARD_OUTLINE_COLOR);
    statsCard.setOutlineThickness(1.f);

    // Card nâng cấp.
    upgradesCard.setPosition(
        sf::Vector2f(
            GameConfig::HUD_LEFT + PANEL_PADDING,
            282.f
        )
    );

    upgradesCard.setSize(
        sf::Vector2f(
            panelWidth - PANEL_PADDING * 2.f,
            150.f
        )
    );

    upgradesCard.setFillColor(CARD_COLOR);
    upgradesCard.setOutlineColor(CARD_OUTLINE_COLOR);
    upgradesCard.setOutlineThickness(1.f);

    // Card hướng dẫn điều khiển.
    helpCard.setPosition(
        sf::Vector2f(
            GameConfig::HUD_LEFT + PANEL_PADDING,
            474.f
        )
    );

    helpCard.setSize(
        sf::Vector2f(
            panelWidth - PANEL_PADDING * 2.f,
            126.f
        )
    );

    helpCard.setFillColor(CARD_COLOR);
    helpCard.setOutlineColor(CARD_OUTLINE_COLOR);
    helpCard.setOutlineThickness(1.f);
}

void HUD::load() {
    fontLoaded = font.openFromFile(
        "assets/fonts/Font.ttf"
    );

    if (!fontLoaded) {
        return;
    }

    // Tiêu đề: nhỏ hơn bản cũ nhưng vẫn nổi bật.
    titleText.emplace(font, "ETERNAL SIEGE", 22);
    titleText->setPosition(
        sf::Vector2f(
            GameConfig::HUD_LEFT + PANEL_PADDING,
            26.f
        )
    );
    titleText->setFillColor(PRIMARY_TEXT_COLOR);

    // Các chỉ số chính.
    statsText.emplace(font, "", 16);
    statsText->setPosition(
        sf::Vector2f(
            GameConfig::HUD_LEFT + PANEL_PADDING + 18.f,
            98.f
        )
    );
    statsText->setFillColor(PRIMARY_TEXT_COLOR);
    statsText->setLineSpacing(1.3f);

    // Tiêu đề phần nâng cấp.
    upgradeTitleText.emplace(font, "UPGRADES", 16);
    upgradeTitleText->setPosition(
        sf::Vector2f(
            GameConfig::HUD_LEFT + PANEL_PADDING,
            246.f
        )
    );
    upgradeTitleText->setFillColor(GOLD_COLOR);

    // Danh sách nâng cấp nhỏ hơn phần chỉ số.
    upgradeText.emplace(font, "", 15);
    upgradeText->setPosition(
        sf::Vector2f(
            GameConfig::HUD_LEFT + PANEL_PADDING + 18.f,
            300.f
        )
    );
    upgradeText->setFillColor(PRIMARY_TEXT_COLOR);
    upgradeText->setLineSpacing(1.45f);

    // Hướng dẫn chỉ cần cỡ 14.
    helpText.emplace(font, "", 14);
    helpText->setPosition(
        sf::Vector2f(
            GameConfig::HUD_LEFT + PANEL_PADDING + 18.f,
            490.f
        )
    );
    helpText->setFillColor(SECONDARY_TEXT_COLOR);
    helpText->setLineSpacing(1.35f);
}

void HUD::update(
    const Player& player,
    const WaveManager& wave,
    const UpgradeManager& upgrades,
    int enemiesAlive
) {
    if (
        !fontLoaded ||
        !statsText ||
        !upgradeText ||
        !helpText
    ) {
        return;
    }

    // ===============================
    // PLAYER AND WAVE INFORMATION
    // ===============================

    std::ostringstream stats;

    stats
        << "HP        "
        << static_cast<int>(player.getHealth())
        << " / "
        << static_cast<int>(player.getMaxHealth())
        << "\n"

        << "GOLD      "
        << upgrades.getGold()
        << "\n"

        << "WAVE      "
        << wave.getCurrentWave()
        << " / "
        << wave.getMaxWaves()
        << "\n"

        << "ENEMIES   "
        << enemiesAlive;

    statsText->setString(stats.str());

    // ===============================
    // UPGRADES
    // ===============================

    std::ostringstream upgradeInfo;

    upgradeInfo
        << "[1] Damage    | Lv."
        << upgrades.getLevel(UpgradeType::Damage)
        << " | "
        << upgrades.getCost(UpgradeType::Damage)
        << " gold\n"

        << "[2] Vitality  | Lv."
        << upgrades.getLevel(UpgradeType::Vitality)
        << " | "
        << upgrades.getCost(UpgradeType::Vitality)
        << " gold\n"

        << "[3] Fire rate | Lv."
        << upgrades.getLevel(UpgradeType::FireRate)
        << " | "
        << upgrades.getCost(UpgradeType::FireRate)
        << " gold";

    upgradeText->setString(upgradeInfo.str());

    // ===============================
    // CONTROLS / WAVE MESSAGE
    // ===============================

    if (wave.isIntermission()) {
        helpText->setString(
            "WAVE CLEARED!\n"
            "Press 1 / 2 / 3 to buy an upgrade\n"
            "BACKSPACE to undo last upgrade\n"
            "Press ENTER to start the next wave"
        );

        helpText->setFillColor(GOLD_COLOR);
    } else {
        helpText->setString(
            "W A S D     Move\n"
            "LEFT MOUSE  Shoot\n"
            "P           Pause"
        );

        helpText->setFillColor(
            SECONDARY_TEXT_COLOR
        );
    }
}

void HUD::draw(sf::RenderWindow& window) const {
    // Vẽ nền trước.
    window.draw(panel);
    window.draw(accentBar);
    window.draw(statsCard);
    window.draw(upgradesCard);
    window.draw(helpCard);

    // Sau đó mới vẽ chữ.
    if (titleText) {
        window.draw(*titleText);
    }

    if (statsText) {
        window.draw(*statsText);
    }

    if (upgradeTitleText) {
        window.draw(*upgradeTitleText);
    }

    if (upgradeText) {
        window.draw(*upgradeText);
    }

    if (helpText) {
        window.draw(*helpText);
    }
}
