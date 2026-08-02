#pragma once

#include "Entity.h"
#include <SFML/Graphics.hpp>
#include "TextureManager.h"
#include <iostream>
#include "Weapon.h"
#include <string_view>

class GameContext;
class Map;
class Effects;

class Player : public Entity {
private:
    float speed = 300.f;
    const sf::Texture* playerTexture;
    sf::RectangleShape playerShape;
    sf::Vector2f playerGripNormalized{0.845f, 0.541f};
    sf::Vector2f previousPosition;
    bool visuallyMoving = false;
    float footstepDistance = 0.f;

    void updatePresentation(Effects* effects);


public:
    static constexpr std::string_view TEXTURE_KEY = "PlayerMage";
    Player(TextureManager& textureManager);

    void handleInput();
    void update(GameContext& context) override;
    void draw(sf::RenderWindow& window) override;
    void drawShadow(sf::RenderWindow& window) const;
    void moveWithCollision(sf::Vector2f displacement, const Map& map);

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
};
