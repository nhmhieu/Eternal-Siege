#pragma once

#include "Weapon.h"
#include <string_view>

class TextureManager;

class SpiritStaff final : public Weapon {
private:
    const sf::Texture* texture = nullptr;
    sf::IntRect visibleBounds{};
    sf::Vector2f gripLocal{};
    sf::Vector2f tipLocal{};
    float textureScale = 1.f;

public:
    static constexpr std::string_view TEXTURE_KEY = "SpiritStaff";
    static constexpr float ATTACK_RANGE = 250.f;
    static constexpr float PROJECTILE_SPEED = 500.f;
    static constexpr float DISPLAY_HEIGHT = 78.f;

    explicit SpiritStaff(const TextureManager& textureManager);

    void draw(sf::RenderWindow& window, sf::Vector2f handAnchor,
              sf::Vector2f direction) const;
    void drawGlow(sf::RenderWindow& window, sf::Vector2f handAnchor,
                  sf::Vector2f direction, float visualTime) const;
    sf::Vector2f getGripPosition(sf::Vector2f handAnchor,
                                 sf::Vector2f direction) const;
    sf::Vector2f getTipPosition(sf::Vector2f handAnchor,
                                sf::Vector2f direction) const;
    bool usesSpriteTexture() const { return texture != nullptr; }
    sf::Vector2f getDisplaySize() const;

    sf::FloatRect getHitbox(sf::Vector2f entityCenter,
                            sf::Vector2f attackDirection) override;
    bool isHitting(sf::Vector2f attackerPosition,
                   sf::Vector2f attackDirection,
                   sf::Vector2f targetPosition) override;
    float getAttackRange() const override { return ATTACK_RANGE; }
    void triggerAction(Entity* attacker, GameContext& context,
                       CombatManager& combatManager) override;
};
