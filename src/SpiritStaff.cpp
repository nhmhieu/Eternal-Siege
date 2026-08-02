#include "SpiritStaff.h"

#include "Effects.h"
#include "Entity.h"
#include "GameContext.h"
#include "Player.h"
#include "SpiritBolt.h"
#include "TextureManager.h"
#include "AudioManager.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>

namespace {
struct StaffGeometry {
    sf::IntRect visible{};
    sf::Vector2f gripLocal{};
    sf::Vector2f tipLocal{};
};

StaffGeometry findStaffGeometry(const sf::Texture& texture) {
    const sf::Image image = texture.copyToImage();
    const sf::Vector2u size = image.getSize();
    if (size.x == 0 || size.y == 0) return {};

    unsigned minX = size.x;
    unsigned minY = size.y;
    unsigned maxX = 0;
    unsigned maxY = 0;
    bool found = false;
    for (unsigned y = 0; y < size.y; ++y) {
        for (unsigned x = 0; x < size.x; ++x) {
            if (image.getPixel({x, y}).a <= 32) continue;
            found = true;
            minX = std::min(minX, x);
            minY = std::min(minY, y);
            maxX = std::max(maxX, x);
            maxY = std::max(maxY, y);
        }
    }
    if (!found) return {};

    const unsigned rawWidth = maxX - minX + 1;
    const unsigned rawHeight = maxY - minY + 1;
    const sf::Vector2f gripPixel{
        static_cast<float>(minX) + 0.500f * rawWidth,
        static_cast<float>(minY) + 0.706f * rawHeight};
    const sf::Vector2f tipPixel{
        static_cast<float>(minX) + 0.500f * rawWidth,
        static_cast<float>(minY)};

    constexpr unsigned padding = 6;
    const unsigned paddedMinX = minX > padding ? minX - padding : 0;
    const unsigned paddedMinY = minY > padding ? minY - padding : 0;
    const unsigned paddedMaxX = std::min(size.x - 1, maxX + padding);
    const unsigned paddedMaxY = std::min(size.y - 1, maxY + padding);
    StaffGeometry result;
    result.visible = {
        {static_cast<int>(paddedMinX), static_cast<int>(paddedMinY)},
        {static_cast<int>(paddedMaxX - paddedMinX + 1),
         static_cast<int>(paddedMaxY - paddedMinY + 1)}};
    result.gripLocal = gripPixel - sf::Vector2f(
        static_cast<float>(paddedMinX), static_cast<float>(paddedMinY));
    result.tipLocal = tipPixel - sf::Vector2f(
        static_cast<float>(paddedMinX), static_cast<float>(paddedMinY));
    return result;
}

float rotationDegrees(sf::Vector2f direction) {
    return std::atan2(direction.y, direction.x) *
               180.f / 3.14159265358979323846f +
           90.f;
}
}

SpiritStaff::SpiritStaff(const TextureManager& textureManager) {
    texture = textureManager.findTexture(std::string(TEXTURE_KEY));
    if (!texture) {
        std::cerr << "SpiritStaff texture is missing: "
                     "assets/images/SpiritStaff.png\n";
        return;
    }
    const StaffGeometry geometry = findStaffGeometry(*texture);
    visibleBounds = geometry.visible;
    gripLocal = geometry.gripLocal;
    tipLocal = geometry.tipLocal;
    if (visibleBounds.size.y <= 0) {
        std::cerr << "SpiritStaff texture has no visible alpha pixels: "
                     "assets/images/SpiritStaff.png\n";
        texture = nullptr;
        return;
    }
    textureScale = DISPLAY_HEIGHT /
                   static_cast<float>(visibleBounds.size.y);
}

void SpiritStaff::draw(sf::RenderWindow& window, sf::Vector2f handAnchor,
                       sf::Vector2f direction) const {
    if (!texture) return;
    sf::Sprite sprite(*texture);
    sprite.setTextureRect(visibleBounds);
    sprite.setOrigin(gripLocal);
    sprite.setPosition(handAnchor);
    sprite.setScale({textureScale, textureScale});
    sprite.setRotation(sf::degrees(rotationDegrees(direction)));
    window.draw(sprite);
}

void SpiritStaff::drawGlow(sf::RenderWindow& window,
                           sf::Vector2f handAnchor,
                           sf::Vector2f direction,
                           float visualTime) const {
    if (!texture) return;
    const sf::Vector2f tip = getTipPosition(handAnchor, direction);
    const float pulse = 1.f + std::sin(visualTime * 4.f) * 0.12f;
    sf::CircleShape outerGlow(20.f);
    outerGlow.setOrigin({20.f, 20.f});
    outerGlow.setPosition(tip);
    outerGlow.setScale({pulse, pulse});
    outerGlow.setFillColor(sf::Color(75, 235, 210, 28));
    window.draw(outerGlow);
    sf::CircleShape glow(13.f);
    glow.setOrigin({13.f, 13.f});
    glow.setPosition(tip);
    glow.setScale({pulse, pulse});
    glow.setFillColor(sf::Color(255, 226, 132, 62));
    window.draw(glow);
}

sf::Vector2f SpiritStaff::getGripPosition(sf::Vector2f handAnchor,
                                          sf::Vector2f direction) const {
    if (!texture) return handAnchor;
    sf::Sprite sprite(*texture);
    sprite.setTextureRect(visibleBounds);
    sprite.setOrigin(gripLocal);
    sprite.setPosition(handAnchor);
    sprite.setScale({textureScale, textureScale});
    sprite.setRotation(sf::degrees(rotationDegrees(direction)));
    return sprite.getTransform().transformPoint(gripLocal);
}

sf::Vector2f SpiritStaff::getTipPosition(sf::Vector2f handAnchor,
                                         sf::Vector2f direction) const {
    if (!texture) return handAnchor;
    sf::Sprite sprite(*texture);
    sprite.setTextureRect(visibleBounds);
    sprite.setOrigin(gripLocal);
    sprite.setPosition(handAnchor);
    sprite.setScale({textureScale, textureScale});
    sprite.setRotation(sf::degrees(rotationDegrees(direction)));
    return sprite.getTransform().transformPoint(tipLocal);
}

sf::Vector2f SpiritStaff::getDisplaySize() const {
    return {static_cast<float>(visibleBounds.size.x) * textureScale,
            static_cast<float>(visibleBounds.size.y) * textureScale};
}

sf::FloatRect SpiritStaff::getHitbox(
    sf::Vector2f entityCenter, sf::Vector2f) {
    return {entityCenter - sf::Vector2f(15.f, 15.f), {30.f, 30.f}};
}

bool SpiritStaff::isHitting(sf::Vector2f attackerPosition,
                            sf::Vector2f,
                            sf::Vector2f targetPosition) {
    const sf::Vector2f offset = targetPosition - attackerPosition;
    return offset.x * offset.x + offset.y * offset.y <=
           ATTACK_RANGE * ATTACK_RANGE;
}

void SpiritStaff::triggerAction(Entity* attacker, GameContext& context,
                                CombatManager&) {
    if (!attacker || attacker->isDead() ||
        !attacker->getIsAttacking() || hasAttacked) {
        return;
    }

    sf::Vector2f origin = attacker->getPosition() +
                          attacker->getAttackDirection() * 30.f;
    if (const auto* player = dynamic_cast<const Player*>(attacker)) {
        origin = player->getSpiritStaffTip();
    }
    context.projectiles.push_back(std::make_unique<SpiritBolt>(
        origin, attacker->getAttackDirection(), PROJECTILE_SPEED,
        attacker->getAttackPower(), attacker->getTeam()));
    if (context.effects) {
        context.effects->spawnSpiritMuzzle(
            origin, attacker->getAttackDirection());
    }
    if (context.audioManager) {
        context.audioManager->playSound("spirit_bolt");
    }
    hasAttacked = true;
}
