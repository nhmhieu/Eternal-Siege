#include "Player.h"
#include "BalanceConfig.h"
#include "GameContext.h"
#include <cmath>
#include "TextureManager.h"
#include "Constants.h"
#include "Map.h"
#include "Effects.h"
#include "SpiritStaff.h"
#include "HeavySpiritBolt.h"
#include <algorithm>
#include <cstdint>
#include <iostream>

namespace {
struct PlayerGeometry {
    sf::IntRect visible{};
    sf::Vector2f gripNormalized{0.845f, 0.541f};
};

PlayerGeometry findPlayerGeometry(const sf::Texture& texture) {
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
        static_cast<float>(minX) + 0.845f * rawWidth,
        static_cast<float>(minY) + 0.541f * rawHeight};
    constexpr unsigned padding = 6;
    minX = minX > padding ? minX - padding : 0;
    minY = minY > padding ? minY - padding : 0;
    maxX = std::min(size.x - 1, maxX + padding);
    maxY = std::min(size.y - 1, maxY + padding);
    PlayerGeometry result;
    result.visible = {{static_cast<int>(minX), static_cast<int>(minY)},
                      {static_cast<int>(maxX - minX + 1),
                       static_cast<int>(maxY - minY + 1)}};
    result.gripNormalized = {
        (gripPixel.x - static_cast<float>(minX)) /
            static_cast<float>(result.visible.size.x),
        (gripPixel.y - static_cast<float>(minY)) /
            static_cast<float>(result.visible.size.y)};
    return result;
}
}

Player::Player(TextureManager& textureManager)
    : Entity(
          400.f,
          300.f,
          BalanceConfig::PLAYER_MAX_HEALTH,
          BalanceConfig::PLAYER_MAX_HEALTH
      ),
      playerTexture(nullptr) {
    team = Team::Player;

    const float desiredHeight = 86.f;

    playerTexture = textureManager.findTexture(std::string(TEXTURE_KEY));
    const PlayerGeometry geometry = playerTexture
        ? findPlayerGeometry(*playerTexture) : PlayerGeometry{};
    const sf::IntRect visible = geometry.visible;
    if (playerTexture && visible.size.x > 0 && visible.size.y > 0) {
        const float aspect = static_cast<float>(visible.size.x) /
                             static_cast<float>(visible.size.y);
        const sf::Vector2f displaySize{
            desiredHeight * aspect, desiredHeight};
        playerShape.setSize(displaySize);
        playerShape.setOrigin({displaySize.x / 2.f, displaySize.y});
        playerShape.setTexture(playerTexture);
        playerShape.setTextureRect(visible);
        playerGripNormalized = geometry.gripNormalized;
    } else {
        std::cerr << "PlayerMage texture is missing; rendering an explicit "
                     "primitive fallback.\n";
        playerShape.setSize({50.f, desiredHeight});
        playerShape.setOrigin({25.f, desiredHeight});
        playerShape.setFillColor(sf::Color(70, 150, 230));
    }
    playerShape.setPosition(sf::Vector2f(400.f, 318.f));
    previousPosition = position;

}


void Player::handleInput() {
    sf::Vector2f movement(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) movement.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) movement.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) movement.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) movement.x += 1.f;

    if (movement.x != 0.f || movement.y != 0.f) {
        float length = std::sqrt(movement.x * movement.x + movement.y * movement.y);
        movement /= length;
    }
    setDirection(movement);

}

void Player::setPosition(const sf::Vector2f& pos) {
    Entity::setPosition(pos.x, pos.y);
    playerShape.setPosition(pos + sf::Vector2f(0.f, 18.f));
}

void Player::setAimDirection(sf::Vector2f aim) {
    const float length = std::sqrt(aim.x * aim.x + aim.y * aim.y);
    if (length > 0.0001f) setAttackDirection(aim / length);
}

sf::Vector2f Player::getSpiritStaffTip() const {
    if (const auto* staff = dynamic_cast<const SpiritStaff*>(
            currentWeapon.get())) {
        return staff->getTipPosition(
            getSpiritStaffAnchor(), attackDirection);
    }
    return getSpiritStaffAnchor();
}

sf::Vector2f Player::getSpiritStaffAnchor() const {
    const sf::Vector2f localGrip{
        playerShape.getSize().x * playerGripNormalized.x,
        playerShape.getSize().y * playerGripNormalized.y};
    return playerShape.getTransform().transformPoint(localGrip);
}

void Player::moveWithCollision(
    sf::Vector2f displacement,
    const Map& map
) {
    constexpr sf::Vector2f collisionHalfExtents{18.f, 18.f};
    setPosition(map.resolveMovement(
        getPosition(), collisionHalfExtents, displacement));
}

void Player::update(GameContext& context) {
    if (context.paused) return;
    Entity::update(context);
    invulnerabilityTimer = std::max(0.f, invulnerabilityTimer - context.deltaTime);
    heavyCooldownTimer = std::max(0.f, heavyCooldownTimer - context.deltaTime);

    if (isDead()) {
        updateDeadTimer(context);
        updatePresentation(context.effects);
        return;
    }

    // X? l input (di chuy?n, t?n cng)
    handleInput();

    // Di chuy?n player
    sf::Vector2f dir = getDirection();
    if (dashing) {
        constexpr float dashSpeed = 900.f;
        constexpr float maxStep = 10.f;
        float distance = dashSpeed * context.deltaTime;
        while (distance > 0.f) {
            const float step = std::min(distance, maxStep);
            if (context.map) moveWithCollision(dashDirection * step, *context.map);
            else setPosition(getPosition() + dashDirection * step);
            distance -= step;
        }
        dashTimer -= context.deltaTime;
        if (dashTimer <= 0.f) dashing = false;
        updateStatus();
        updatePresentation(context.effects);
        return;
    }
    if (dir.x != 0.f || dir.y != 0.f) {
        const sf::Vector2f displacement =
            dir * speed * (chargingHeavy ? 0.45f : 1.f) * context.deltaTime;
        if (context.map) {
            moveWithCollision(displacement, *context.map);
        } else {
            setPosition(getPosition() + displacement);
        }
    }

    // X? l t?n cng
    if (isAttacking) {
        updateAttackTimer(context);
        if (currentWeapon && context.combatManager) {
            currentWeapon->triggerAction(this, context, *context.combatManager);
        }
    }

    // C?p nh?t tr?ng thi t?n cng (t? t?t)
    updateStatus();
    updatePresentation(context.effects);
}

void Player::beginDash(sf::Vector2f move, sf::Vector2f aim, Effects* effects) {
    sf::Vector2f chosen = move;
    float length = std::sqrt(chosen.x * chosen.x + chosen.y * chosen.y);
    if (length <= 0.0001f) { chosen = aim; length = std::sqrt(aim.x*aim.x + aim.y*aim.y); }
    if (length <= 0.0001f) { chosen = attackDirection; length = std::sqrt(chosen.x*chosen.x + chosen.y*chosen.y); }
    dashDirection = length > 0.0001f ? chosen / length : sf::Vector2f{1.f, 0.f};
    chargingHeavy = false;
    isAttacking = false;
    dashing = true;
    dashTimer = 0.15f;
    invulnerabilityTimer = std::max(invulnerabilityTimer, 0.17f);
    if (effects) effects->spawnImpact(position, ImpactStyle::Spirit);
}

void Player::setHeavyCharging(bool charging, float ratio) {
    chargingHeavy = charging;
    chargeRatio = std::clamp(ratio, 0.f, 1.f);
}

bool Player::releaseHeavy(GameContext& context, float ratio) {
    chargingHeavy = false;
    if (heavyCooldownTimer > 0.f || isDead()) return false;
    ratio = std::clamp(ratio, 0.f, 1.f);
    const float multiplier = 1.8f + (3.f - 1.8f) * ratio;
    const float damage = getAttackPower() * multiplier;
    context.projectiles.push_back(std::make_unique<HeavySpiritBolt>(
        getSpiritStaffTip(), attackDirection, damage, team, damage * 0.40f, 72.f));
    heavyCooldownTimer = std::max(0.85f, getAttackCooldown() * 1.8f);
    if (context.effects) {
        context.effects->spawnSpiritMuzzle(getSpiritStaffTip(), attackDirection);
        context.effects->requestScreenShake(3.f);
    }
    return true;
}

void Player::takeDamage(float damage) {
    if (invulnerabilityTimer > 0.f) return;
    Entity::takeDamage(damage);
}

void Player::draw(sf::RenderWindow& window) {
    const auto* staff = dynamic_cast<const SpiritStaff*>(currentWeapon.get());
    const sf::Vector2f handAnchor = getSpiritStaffAnchor();
    if (staff) staff->draw(window, handAnchor, attackDirection);
    window.draw(playerShape);
    if (staff) {
        staff->drawGlow(window, handAnchor, attackDirection, visualTime);
    }
    if (chargingHeavy) {
        const float radius = 10.f + chargeRatio * 13.f;
        sf::CircleShape chargeRing(radius);
        chargeRing.setOrigin({radius, radius});
        chargeRing.setPosition(getSpiritStaffTip());
        chargeRing.setFillColor(sf::Color::Transparent);
        chargeRing.setOutlineThickness(2.f + chargeRatio * 2.f);
        chargeRing.setOutlineColor(chargeRatio >= 1.f
            ? sf::Color(255, 220, 95, 245)
            : sf::Color(72, 235, 202, 185));
        window.draw(chargeRing);
    }
}

void Player::drawShadow(sf::RenderWindow& window) const {
    const float shadowRadius = std::max(14.f, playerShape.getSize().x * 0.24f);
    sf::CircleShape shadow(shadowRadius);
    shadow.setOrigin({shadowRadius, shadowRadius});
    shadow.setScale({1.25f, 0.36f});
    shadow.setPosition({position.x, position.y + 2.f});
    shadow.setFillColor(sf::Color(8, 5, 15, isDying ? 30 : 65));
    window.draw(shadow);
}

void Player::updateNonCombatPresentation(float deltaTime,
                                         sf::Vector2f movement) {
    const sf::Vector2f actual=position-previousPosition;
    visualTime += std::max(0.f, deltaTime);
    setDirection(movement);
    updatePresentation(nullptr);
    if(walkTexture){const bool moving=std::abs(actual.x)+std::abs(actual.y)>.001f;if(moving){FacingDirection next=walkFacing;if(std::abs(movement.x)>std::abs(movement.y))next=movement.x<0?FacingDirection::Left:FacingDirection::Right;else next=movement.y<0?FacingDirection::Up:FacingDirection::Down;if(next!=walkFacing){walkFacing=next;AnimationClip clip;for(int i=0;i<6;++i)clip.frames.push_back({{i*256,static_cast<int>(walkFacing)*256},{256,256}});walkAnimation.setClip(std::move(clip));}walkAnimation.update(deltaTime);}else walkAnimation.reset();if(const auto* frame=walkAnimation.currentFrame())playerShape.setTextureRect(*frame);}
}

void Player::useWalkSpriteSheet(const sf::Texture& texture){walkTexture=&texture;playerShape.setTexture(walkTexture,true);AnimationClip clip;for(int i=0;i<6;++i)clip.frames.push_back({{i*256,0},{256,256}});walkAnimation.setClip(std::move(clip));playerShape.setTextureRect({{0,0},{256,256}});playerShape.setSize({86.f,86.f});playerShape.setOrigin({43.f,86.f});}

void Player::updatePresentation(Effects* effects) {
    const sf::Vector2f moved = position - previousPosition;
    const float movedDistance = std::sqrt(
        moved.x * moved.x + moved.y * moved.y);
    visuallyMoving = movedDistance > 0.001f;
    footstepDistance += movedDistance;
    if (effects && footstepDistance >= 22.f) {
        effects->spawnFootstep(
            position + sf::Vector2f(0.f, 18.f), FootstepStyle::Player);
        footstepDistance = std::fmod(footstepDistance, 22.f);
    }
    previousPosition = position;

    const float bob = visuallyMoving
        ? std::abs(std::sin(visualTime * 12.f)) * 2.f
        : std::sin(visualTime * 3.2f) * 0.7f;
    float rotation = visuallyMoving ? std::sin(visualTime * 12.f) : 0.f;
    sf::Vector2f visualPosition = position - sf::Vector2f(0.f, bob);
    float scaleX = 1.f;
    float scaleY = 1.f;

    if (isAttacking) {
        const float progress = getAttackAnimationProgress();
        const float pulse = progress <= 0.10f
            ? std::sin((progress / 0.10f) * 3.14159265358979323846f)
            : 0.f;
        visualPosition -= attackDirection * 4.f * pulse;
        scaleX += 0.08f * pulse;
        scaleY -= 0.05f * pulse;
        rotation -= attackDirection.x * 5.f * pulse;
    }

    const float facing = (direction.x < -0.05f ||
        (direction.x == 0.f && attackDirection.x < -0.05f)) ? -1.f : 1.f;
    sf::Color tint = playerTexture
        ? sf::Color::White
        : sf::Color(70, 150, 230);
    if (isDying) {
        const float progress = std::clamp(
            deadTimer / std::max(0.01f, deadAnimationDuration), 0.f, 1.f);
        rotation += facing * 75.f * progress;
        scaleX *= 1.f - progress * 0.25f;
        scaleY *= 1.f - progress * 0.45f;
        tint.a = static_cast<std::uint8_t>(255.f * (1.f - progress));
    } else if (healingFlashTimer > 0.f) {
        tint = sf::Color(125, 255, 205);
    } else if (hurtFlashTimer > 0.f) {
        tint = sf::Color(255, 135, 135);
    }

    playerShape.setPosition(visualPosition);
    playerShape.setScale({facing * scaleX, scaleY});
    playerShape.setRotation(sf::degrees(rotation));
    playerShape.setFillColor(tint);
}

sf::FloatRect Player::getCollisionBox() const {
    constexpr sf::Vector2f collisionSize{36.f, 36.f};
    return {getPosition() - collisionSize / 2.f, collisionSize};
}

sf::FloatRect Player::getHurtBox() const {
    constexpr sf::Vector2f hurtSize{80.f, 80.f};
    return {getPosition() - hurtSize / 2.f, hurtSize};
}
