#include "SpiritBolt.h"

SpiritBolt::SpiritBolt(sf::Vector2f startPosition,
                       sf::Vector2f direction,
                       float speed,
                       float damage,
                       Team shooterTeam)
    : Projectiles(startPosition, direction, speed, damage, shooterTeam,
                  ProjectileStyle::Spirit) {
    configureVisual({16.f, 10.f}, sf::Color(78, 238, 204));
}
