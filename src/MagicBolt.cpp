#include "MagicBolt.h"

MagicBolt::MagicBolt(sf::Vector2f startPosition,
                     sf::Vector2f direction,
                     float speed,
                     float damage,
                     Team shooterTeam)
    : Projectiles(startPosition, direction, speed, damage, shooterTeam,
                  ProjectileStyle::Magic) {
    configureVisual({14.f, 14.f}, sf::Color(238, 188, 255));
}
