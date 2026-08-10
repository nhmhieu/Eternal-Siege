#include "Effects.h"

#include "AssetLocator.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <utility>

namespace {
constexpr float PI = 3.14159265358979323846f;

std::uint8_t fadedAlpha(std::uint8_t alpha, float factor) {
    return static_cast<std::uint8_t>(
        std::clamp(static_cast<float>(alpha) * factor, 0.f, 255.f));
}
}

Effects::Effects() {
    const auto path = AssetLocator::find("assets/fonts/Font.ttf");
    fontLoaded = path && font.openFromFile(*path);
}

void Effects::clear() {
    particles.clear();
    rings.clear();
    slashes.clear();
    beams.clear();
    floatingTexts.clear();
    pendingShakeStrength = 0.f;
    skillCastEvents = 0;
}

float Effects::randomFloat(float minValue, float maxValue) {
    std::uniform_real_distribution<float> distribution(minValue, maxValue);
    return distribution(random);
}

void Effects::pushParticle(Particle particle) {
    if (particles.size() >= MAX_PARTICLES) particles.erase(particles.begin());
    particles.push_back(std::move(particle));
}

void Effects::enforceEffectInstanceLimit() {
    while (rings.size() + slashes.size() + beams.size() >
           MAX_EFFECT_INSTANCES) {
        if (!rings.empty()) rings.erase(rings.begin());
        else if (!slashes.empty()) slashes.erase(slashes.begin());
        else if (!beams.empty()) beams.erase(beams.begin());
    }
}

void Effects::pushRing(Ring ring) {
    rings.push_back(std::move(ring));
    enforceEffectInstanceLimit();
}

void Effects::pushSlash(Slash slash) {
    slashes.push_back(std::move(slash));
    enforceEffectInstanceLimit();
}

void Effects::pushBeam(Beam beam) {
    beams.push_back(std::move(beam));
    enforceEffectInstanceLimit();
}

void Effects::pushFloatingText(FloatingText text) {
    if (floatingTexts.size() >= MAX_FLOATING_TEXTS) {
        floatingTexts.erase(floatingTexts.begin());
    }
    floatingTexts.push_back(std::move(text));
}

void Effects::addBurst(sf::Vector2f position, sf::Color color, int count,
                       float minSpeed, float maxSpeed, float minRadius,
                       float maxRadius, float life, float gravity) {
    for (int index = 0; index < count; ++index) {
        const float angle = randomFloat(0.f, PI * 2.f);
        const float speed = randomFloat(minSpeed, maxSpeed);
        Particle particle;
        particle.position = position;
        particle.velocity = {std::cos(angle) * speed, std::sin(angle) * speed};
        particle.color = color;
        particle.radius = randomFloat(minRadius, maxRadius);
        particle.life = randomFloat(life * 0.72f, life);
        particle.maxLife = particle.life;
        particle.drag = 2.4f;
        particle.gravity = gravity;
        pushParticle(std::move(particle));
    }
}

void Effects::spawnSlash(sf::Vector2f position, sf::Vector2f direction,
                         bool enemyAttack) {
    spawnSlash(position, direction,
               enemyAttack ? sf::Color(255, 88, 82, 225)
                           : sf::Color(255, 229, 145, 235),
               1.f);
}

void Effects::spawnSlash(sf::Vector2f position, sf::Vector2f direction,
                         sf::Color color, float scale) {
    Slash slash;
    slash.position = position + direction * (15.f * scale);
    slash.direction = direction;
    slash.color = color;
    slash.scale = scale;
    slash.life = 0.18f + 0.04f * (scale - 1.f);
    slash.maxLife = slash.life;
    pushSlash(std::move(slash));

    const sf::Vector2f perpendicular{-direction.y, direction.x};
    const int sparkCount = std::max(3, static_cast<int>(5.f * scale));
    for (int index = 0; index < sparkCount; ++index) {
        Particle spark;
        spark.position = position + direction * 15.f +
                         perpendicular * randomFloat(-9.f, 9.f) * scale;
        spark.velocity = direction * randomFloat(60.f, 130.f) +
                         perpendicular * randomFloat(-35.f, 35.f);
        spark.color = color;
        spark.radius = randomFloat(1.5f, 3.3f) * scale;
        spark.life = randomFloat(0.12f, 0.24f);
        spark.maxLife = spark.life;
        spark.drag = 5.f;
        pushParticle(std::move(spark));
    }
}

void Effects::spawnShot(sf::Vector2f position, sf::Vector2f direction,
                        bool magic) {
    const sf::Color color = magic
        ? sf::Color(201, 100, 255, 235)
        : sf::Color(255, 218, 110, 225);
    const sf::Vector2f perpendicular{-direction.y, direction.x};
    for (int index = 0; index < (magic ? 11 : 6); ++index) {
        Particle particle;
        particle.position = position + direction * 11.f +
                            perpendicular * randomFloat(-6.f, 6.f);
        particle.velocity = direction * randomFloat(55.f, 130.f) +
                            perpendicular * randomFloat(-55.f, 55.f);
        particle.color = color;
        particle.radius = randomFloat(1.3f, magic ? 4.2f : 2.6f);
        particle.life = randomFloat(0.12f, magic ? 0.34f : 0.22f);
        particle.maxLife = particle.life;
        particle.drag = 5.f;
        pushParticle(std::move(particle));
    }
    pushRing({position + direction * 10.f, color, 3.f,
              magic ? 21.f : 13.f, 0.18f, 0.18f, 2.f});
}

void Effects::spawnSpiritMuzzle(sf::Vector2f position,
                                sf::Vector2f direction) {
    const sf::Color teal(76, 240, 210, 240);
    const sf::Color gold(255, 226, 132, 225);
    addBurst(position, teal, 8, 40.f, 120.f, 1.4f, 3.4f, 0.28f);
    addBurst(position, gold, 5, 30.f, 90.f, 1.1f, 2.5f, 0.22f);
    pushRing({position + direction * 3.f, gold, 3.f, 22.f,
              0.22f, 0.22f, 2.2f});
}

void Effects::spawnImpact(sf::Vector2f position, ImpactStyle style) {
    sf::Color color;
    int count = 9;
    float radius = 22.f;
    switch (style) {
    case ImpactStyle::Magic:
        color = sf::Color(205, 100, 255, 240);
        count = 14;
        radius = 30.f;
        break;
    case ImpactStyle::Spirit:
        color = sf::Color(78, 238, 204, 240);
        count = 15;
        radius = 34.f;
        break;
    case ImpactStyle::Strong:
        color = sf::Color(255, 125, 45, 240);
        count = 17;
        radius = 38.f;
        break;
    case ImpactStyle::Enemy:
        color = sf::Color(255, 82, 72, 235);
        break;
    case ImpactStyle::Physical:
    default:
        color = sf::Color(255, 220, 135, 235);
        break;
    }
    addBurst(position, color, count, 45.f, 155.f, 1.4f, 4.3f,
             style == ImpactStyle::Strong ? 0.48f : 0.36f, 40.f);
    pushRing({position, color, 4.f, radius, 0.28f, 0.28f,
              style == ImpactStyle::Strong ? 3.5f : 2.5f});
    if (style == ImpactStyle::Spirit) {
        pushRing({position, sf::Color(255, 226, 132, 210), 7.f, 26.f,
                  0.22f, 0.22f, 1.8f});
    }
}

void Effects::spawnDeath(sf::Vector2f position, bool boss) {
    const sf::Color color = boss
        ? sf::Color(229, 90, 255, 240)
        : sf::Color(235, 70, 76, 230);
    addBurst(position, color, boss ? 38 : 18, 45.f, boss ? 245.f : 165.f,
             2.f, boss ? 6.f : 4.f, boss ? 0.85f : 0.55f, 65.f);
    pushRing({position, color, 8.f, boss ? 88.f : 42.f,
              boss ? 0.62f : 0.38f, boss ? 0.62f : 0.38f,
              boss ? 5.f : 3.f});
    if (boss) requestScreenShake(5.f);
}

void Effects::spawnPortal(sf::Vector2f position, bool boss) {
    const sf::Color color = boss
        ? sf::Color(225, 90, 255, 220)
        : sf::Color(255, 115, 90, 190);
    pushRing({position, color, boss ? 12.f : 8.f, boss ? 92.f : 44.f,
              boss ? 0.75f : 0.42f, boss ? 0.75f : 0.42f,
              boss ? 5.f : 3.f});
    addBurst(position, color, boss ? 30 : 12, 20.f, boss ? 150.f : 90.f,
             1.4f, boss ? 5.f : 3.2f, boss ? 0.75f : 0.45f, -20.f);
}

void Effects::spawnFootstep(sf::Vector2f position, FootstepStyle style) {
    sf::Color color(190, 180, 165, 115);
    int count = 3;
    float scale = 1.f;
    switch (style) {
    case FootstepStyle::Player:
        color = sf::Color(110, 225, 215, 125);
        break;
    case FootstepStyle::Ally:
        color = sf::Color(205, 205, 190, 100);
        break;
    case FootstepStyle::Normal:
        color = sf::Color(180, 75, 115, 105);
        break;
    case FootstepStyle::Elite:
        color = sf::Color(150, 65, 120, 125);
        count = 5;
        scale = 1.35f;
        break;
    case FootstepStyle::Boss:
        color = sf::Color(75, 35, 95, 145);
        count = 8;
        scale = 1.8f;
        pushRing({position, color, 5.f, 30.f, 0.38f, 0.38f, 2.f, true});
        break;
    }
    for (int index = 0; index < count; ++index) {
        const float angle = randomFloat(0.f, PI * 2.f);
        const float speed = randomFloat(8.f, 35.f);
        Particle particle;
        particle.position = position;
        particle.velocity = {std::cos(angle) * speed,
                             std::sin(angle) * speed * 0.45f};
        particle.color = color;
        particle.radius = randomFloat(1.4f, 2.8f) * scale;
        particle.life = randomFloat(0.36f, 0.48f);
        particle.maxLife = particle.life;
        particle.drag = 4.f;
        particle.gravity = -8.f;
        particle.ground = true;
        pushParticle(std::move(particle));
    }
}

void Effects::spawnAllySkill(sf::Vector2f position, sf::Color color,
                             float radius, const std::string& name) {
    ++skillCastEvents;
    pushRing({position, color, 8.f, radius, 0.48f, 0.48f, 4.f, true});
    pushRing({position, sf::Color(color.r, color.g, color.b, 150),
              radius * 0.28f, radius * 0.72f, 0.38f, 0.38f, 2.f, true});
    addBurst(position, color, 20, 25.f, 120.f, 1.5f, 4.f, 0.52f, -18.f);
    if (name == "Shield Wall") {
        pushRing({position, sf::Color(150, 235, 255, 210), radius * .72f,
                  radius, .75f, .75f, 6.f, false});
    } else if (name == "Arcane Burst") {
        pushRing({position, sf::Color(190, 90, 255, 190), radius * .12f,
                  radius, .62f, .62f, 5.f, true});
        addBurst(position, sf::Color(105, 225, 255), 12, 12.f, 75.f,
                 1.f, 2.8f, .7f, -12.f);
    } else if (name == "Whirlwind") {
        pushRing({position, sf::Color(255, 205, 110, 175), radius * .45f,
                  radius, .68f, .68f, 3.f, true});
        pushRing({position, sf::Color(245, 105, 55, 135), radius * .25f,
                  radius * .82f, .48f, .48f, 2.f, false});
    } else if (name == "Explosive Arrow") {
        pushRing({position, sf::Color(255, 225, 125, 190), 5.f,
                  radius * .75f, .35f, .35f, 5.f, false});
    }
    FloatingText label;
    label.position = position - sf::Vector2f(0.f, 48.f);
    label.value = name;
    label.color = color;
    label.size = 16;
    label.life = 0.5f;
    label.maxLife = 0.5f;
    pushFloatingText(std::move(label));
}

void Effects::spawnRadiantPulse(sf::Vector2f position, float radius) {
    const sf::Color teal(74, 235, 205, 225);
    pushRing({position, teal, 12.f, radius, 0.72f, 0.72f, 5.f, true});
    pushRing({position, sf::Color(255, 225, 125, 190), 24.f,
              radius * 0.72f, 0.58f, 0.58f, 2.5f, true});
    addBurst(position, teal, 32, 35.f, 155.f, 1.5f, 4.2f, 0.68f, -35.f);
    pushRing({position, sf::Color(255, 250, 210, 225), 4.f,
              radius * .22f, .22f, .22f, 7.f, false});
    pushRing({position, sf::Color(75, 210, 190, 100), radius * .45f,
              radius * 1.08f, .9f, .9f, 2.f, true});
    requestScreenShake(1.5f);
}

void Effects::spawnHealLink(sf::Vector2f start, sf::Vector2f end) {
    pushBeam({start, end, sf::Color(110, 255, 205, 205),
              0.32f, 0.32f});
    const sf::Vector2f delta = end - start;
    for (int index = 1; index <= 5; ++index) {
        Particle particle;
        particle.position = start + delta * (static_cast<float>(index) / 6.f);
        particle.velocity = {randomFloat(-8.f, 8.f), randomFloat(-35.f, -15.f)};
        particle.color = sf::Color(255, 232, 145, 205);
        particle.radius = randomFloat(1.5f, 3.f);
        particle.life = 0.42f;
        particle.maxLife = particle.life;
        particle.drag = 2.f;
        pushParticle(std::move(particle));
    }
}

void Effects::spawnFloatingNumber(sf::Vector2f position, float amount,
                                  bool healing, bool strong) {
    if (amount <= 0.f) return;
    const int rounded = std::max(1, static_cast<int>(std::lround(amount)));
    FloatingText text;
    text.position = position - sf::Vector2f(0.f, 28.f);
    text.value = healing ? "+" + std::to_string(rounded)
                         : std::to_string(rounded);
    text.color = healing
        ? sf::Color(85, 245, 185)
        : (strong ? sf::Color(255, 145, 55)
                  : sf::Color(255, 238, 185));
    text.size = strong ? 21u : 18u;
    text.life = strong ? 0.9f : 0.75f;
    text.maxLife = text.life;
    pushFloatingText(std::move(text));
}

void Effects::requestScreenShake(float strength) {
    pendingShakeStrength = std::max(pendingShakeStrength, strength);
}

float Effects::consumeScreenShakeRequest() {
    const float request = pendingShakeStrength;
    pendingShakeStrength = 0.f;
    return request;
}

void Effects::update(float deltaTime, bool paused) {
    if (paused) return;
    for (auto& particle : particles) {
        particle.life -= deltaTime;
        const float damping = std::max(0.f, 1.f - particle.drag * deltaTime);
        particle.velocity *= damping;
        particle.velocity.y += particle.gravity * deltaTime;
        particle.position += particle.velocity * deltaTime;
    }
    particles.erase(std::remove_if(particles.begin(), particles.end(),
        [](const Particle& particle) { return particle.life <= 0.f; }),
        particles.end());

    for (auto& ring : rings) ring.life -= deltaTime;
    rings.erase(std::remove_if(rings.begin(), rings.end(),
        [](const Ring& ring) { return ring.life <= 0.f; }), rings.end());

    for (auto& slash : slashes) slash.life -= deltaTime;
    slashes.erase(std::remove_if(slashes.begin(), slashes.end(),
        [](const Slash& slash) { return slash.life <= 0.f; }), slashes.end());

    for (auto& beam : beams) beam.life -= deltaTime;
    beams.erase(std::remove_if(beams.begin(), beams.end(),
        [](const Beam& beam) { return beam.life <= 0.f; }), beams.end());

    for (auto& text : floatingTexts) {
        text.life -= deltaTime;
        text.position.y -= 34.f * deltaTime;
    }
    floatingTexts.erase(std::remove_if(
        floatingTexts.begin(), floatingTexts.end(),
        [](const FloatingText& text) { return text.life <= 0.f; }),
        floatingTexts.end());
}

void Effects::drawGround(sf::RenderWindow& window) const {
    for (const auto& ring : rings) {
        if (!ring.ground) continue;
        const float progress = 1.f - ring.life / ring.maxLife;
        const float radius = ring.startRadius +
                             (ring.endRadius - ring.startRadius) * progress;
        sf::CircleShape shape(radius);
        shape.setOrigin({radius, radius});
        shape.setPosition(ring.position);
        shape.setFillColor(sf::Color::Transparent);
        sf::Color color = ring.color;
        color.a = fadedAlpha(color.a, 1.f - progress);
        shape.setOutlineColor(color);
        shape.setOutlineThickness(ring.thickness * (1.f - progress * 0.55f));
        window.draw(shape);
    }

    for (const auto& particle : particles) {
        if (!particle.ground) continue;
        const float remaining = particle.life / particle.maxLife;
        sf::CircleShape shape(particle.radius * (0.55f + remaining * 0.45f));
        const float radius = shape.getRadius();
        shape.setOrigin({radius, radius});
        shape.setPosition(particle.position);
        sf::Color color = particle.color;
        color.a = fadedAlpha(color.a, remaining);
        shape.setFillColor(color);
        window.draw(shape);
    }
}

void Effects::drawCombat(sf::RenderWindow& window) const {
    for (const auto& ring : rings) {
        if (ring.ground) continue;
        const float progress = 1.f - ring.life / ring.maxLife;
        const float radius = ring.startRadius +
                             (ring.endRadius - ring.startRadius) * progress;
        sf::CircleShape shape(radius);
        shape.setOrigin({radius, radius});
        shape.setPosition(ring.position);
        shape.setFillColor(sf::Color::Transparent);
        sf::Color color = ring.color;
        color.a = fadedAlpha(color.a, 1.f - progress);
        shape.setOutlineColor(color);
        shape.setOutlineThickness(ring.thickness * (1.f - progress * 0.55f));
        window.draw(shape);
    }

    for (const auto& beam : beams) {
        const sf::Vector2f delta = beam.end - beam.start;
        const float length = std::sqrt(delta.x * delta.x + delta.y * delta.y);
        if (length <= 0.001f) continue;
        sf::RectangleShape line({length, 3.f});
        line.setOrigin({0.f, 1.5f});
        line.setPosition(beam.start);
        line.setRotation(sf::radians(std::atan2(delta.y, delta.x)));
        sf::Color color = beam.color;
        color.a = fadedAlpha(color.a, beam.life / beam.maxLife);
        line.setFillColor(color);
        window.draw(line);
    }

    for (const auto& slash : slashes) {
        const float progress = 1.f - slash.life / slash.maxLife;
        const float angle = std::atan2(slash.direction.y, slash.direction.x);
        const sf::Vector2f perpendicular{-slash.direction.y, slash.direction.x};
        for (int index = -1; index <= 1; ++index) {
            sf::RectangleShape streak({
                (54.f + 8.f * (1.f - progress)) * slash.scale,
                (index == 0 ? 4.f : 2.f) * slash.scale});
            streak.setOrigin({6.f, index == 0 ? 2.f : 1.f});
            streak.setPosition(slash.position + perpendicular *
                               static_cast<float>(index * 7) * slash.scale);
            streak.setRotation(sf::radians(angle +
                static_cast<float>(index) * 0.12f));
            sf::Color color = slash.color;
            color.a = fadedAlpha(color.a, 1.f - progress);
            streak.setFillColor(color);
            window.draw(streak);
        }
    }

    for (const auto& particle : particles) {
        if (particle.ground) continue;
        const float remaining = particle.life / particle.maxLife;
        sf::CircleShape shape(particle.radius * (0.55f + remaining * 0.45f));
        const float radius = shape.getRadius();
        shape.setOrigin({radius, radius});
        shape.setPosition(particle.position);
        sf::Color color = particle.color;
        color.a = fadedAlpha(color.a, remaining);
        shape.setFillColor(color);
        window.draw(shape);
    }
}

void Effects::drawFloating(sf::RenderWindow& window) const {
    if (fontLoaded) {
        for (const auto& floating : floatingTexts) {
            sf::Text text(font, floating.value, floating.size);
            const sf::FloatRect bounds = text.getLocalBounds();
            text.setOrigin({bounds.position.x + bounds.size.x / 2.f,
                            bounds.position.y + bounds.size.y / 2.f});
            text.setPosition(floating.position);
            sf::Color color = floating.color;
            color.a = fadedAlpha(color.a, floating.life / floating.maxLife);
            text.setFillColor(color);
            text.setOutlineColor(sf::Color(15, 18, 24, color.a));
            text.setOutlineThickness(1.2f);
            window.draw(text);
        }
    }
}

void Effects::draw(sf::RenderWindow& window) const {
    drawGround(window);
    drawCombat(window);
    drawFloating(window);
}
