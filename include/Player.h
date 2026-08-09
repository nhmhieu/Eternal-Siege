#pragma once

#include "Entity.h"
#include <SFML/Graphics.hpp>
#include "TextureManager.h"
#include <iostream>
#include "Weapon.h"
#include "AnimationController.h"
#include <string_view>

class GameContext;
class Map;
class Effects;
class PlayerActionController;

class Player : public Entity {
private:
    float speed = 300.f;
    const sf::Texture* playerTexture;
    sf::RectangleShape playerShape;
    sf::Vector2f playerGripNormalized{0.845f, 0.541f};
    sf::Vector2f previousPosition;
    bool visuallyMoving = false;
    float footstepDistance = 0.f;
    bool dashing = false;
    bool chargingHeavy = false;
    float dashTimer = 0.f;
    float invulnerabilityTimer = 0.f;
    float heavyCooldownTimer = 0.f;
    float chargeRatio = 0.f;
    sf::Vector2f dashDirection{1.f, 0.f};
    AnimationController walkAnimation;
    const sf::Texture* walkTexture = nullptr;
    FacingDirection walkFacing = FacingDirection::Down;

    void updatePresentation(Effects* effects);


public:
    static constexpr std::string_view TEXTURE_KEY = "PlayerMage";
    Player(TextureManager& textureManager);

    void handleInput();
    void update(GameContext& context) override;
    void draw(sf::RenderWindow& window) override;
    void drawShadow(sf::RenderWindow& window) const;
    void updateNonCombatPresentation(float deltaTime, sf::Vector2f movement);
    void useWalkSpriteSheet(const sf::Texture& texture);
    void moveWithCollision(sf::Vector2f displacement, const Map& map);
    void beginDash(sf::Vector2f movementDirection, sf::Vector2f aimDirection,
                   Effects* effects);
    bool releaseHeavy(GameContext& context, float ratio);
    void setHeavyCharging(bool charging, float ratio = 0.f);
    void takeDamage(float damage) override;
    bool isInvulnerable() const { return invulnerabilityTimer > 0.f; }
    float getHeavyCooldownRemaining() const { return heavyCooldownTimer; }
    bool isHeavyReady() const { return heavyCooldownTimer <= 0.f; }
    bool isDashing() const { return dashing; }

    bool canAttack() {
        return !isAttacking && coolDownTimer <= 0.f;
    }
    void setAimDirection(sf::Vector2f direction);
    sf::Vector2f getSpiritStaffAnchor() const;
    sf::Vector2f getSpiritStaffTip() const;
    sf::Vector2f getVisualSize() const { return playerShape.getSize(); }
    void setPosition(const sf::Vector2f& pos);

    sf::FloatRect getCollisionBox() const override;
    sf::FloatRect getHurtBox() const override;
    sf::Vector2f getFootPosition() const { return position; }
    sf::FloatRect getFootCollider() const { return {{position.x-14.f,position.y-8.f},{28.f,16.f}}; }
    void setPresentationTint(sf::Color color) { playerShape.setFillColor(color); }
};
