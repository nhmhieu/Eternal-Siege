#include "HeavySpiritBolt.h"

HeavySpiritBolt::HeavySpiritBolt(sf::Vector2f start, sf::Vector2f direction,
                                 float damage, Team team, float splashDamage,
                                 float splashRadius)
    : Projectiles(start, direction, 420.f, damage, team,
                  ProjectileStyle::HeavySpirit) {
    configureVisual({28.f, 18.f}, sf::Color(98, 244, 202));
    configureExplosion(splashDamage, splashRadius);
}
