#include "Player.h"

#include <cmath>

Player::Player()
{
    sprite.setRadius(20.f);
    sprite.setFillColor(sf::Color::Cyan);

    sprite.setOrigin({
        sprite.getRadius(),
        sprite.getRadius()
    });

    sprite.setPosition({
        300.f,
        300.f
    });

    speed = 250.f;
}

void Player::handleInput()
{
}

void Player::setPosition(const sf::Vector2f& pos)
{
    sprite.setPosition(pos);
}

void Player::update(float dt)
{
    sf::Vector2f movement(0.f, 0.f);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
        movement.y--;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
        movement.y++;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
        movement.x--;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
        movement.x++;

    if (movement.x != 0.f || movement.y != 0.f)
    {
        float len = std::sqrt(
            movement.x * movement.x +
            movement.y * movement.y);

        movement /= len;

        sprite.move(movement * speed * dt);
    }
}

void Player::render(sf::RenderWindow& window)
{
    window.draw(sprite);
}