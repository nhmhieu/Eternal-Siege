#pragma once

#include <SFML/Graphics.hpp>

#include <cstdint>
#include <random>
#include <string>
#include <vector>

enum class ImpactStyle {
    Physical,
    Magic,
    Spirit,
    Strong,
    Enemy
};

enum class FootstepStyle {
    Player,
    Ally,
    Normal,
    Elite,
    Boss
};

class Effects {
public:
    static constexpr std::size_t MAX_PARTICLES = 600;
    static constexpr std::size_t MAX_EFFECT_INSTANCES = 128;
    static constexpr std::size_t MAX_FLOATING_TEXTS = 100;

    Effects();

    void clear();
    void update(float deltaTime, bool paused = false);
    void draw(sf::RenderWindow& window) const;
    void drawGround(sf::RenderWindow& window) const;
    void drawCombat(sf::RenderWindow& window) const;
    void drawFloating(sf::RenderWindow& window) const;

    void spawnSlash(sf::Vector2f position, sf::Vector2f direction,
                    bool enemyAttack);
    void spawnSlash(sf::Vector2f position, sf::Vector2f direction,
                    sf::Color color, float scale);
    void spawnShot(sf::Vector2f position, sf::Vector2f direction,
                   bool magic);
    void spawnSpiritMuzzle(sf::Vector2f position, sf::Vector2f direction);
    void spawnImpact(sf::Vector2f position, ImpactStyle style);
    void spawnDeath(sf::Vector2f position, bool boss);
    void spawnPortal(sf::Vector2f position, bool boss);
    void spawnFootstep(sf::Vector2f position, FootstepStyle style);
    void spawnAllySkill(sf::Vector2f position, sf::Color color,
                        float radius, const std::string& name);
    void spawnRadiantPulse(sf::Vector2f position, float radius);
    void spawnHealLink(sf::Vector2f start, sf::Vector2f end);
    void spawnFloatingNumber(sf::Vector2f position, float amount,
                             bool healing, bool strong = false);

    void requestScreenShake(float strength);
    float consumeScreenShakeRequest();

    std::size_t particleCount() const { return particles.size(); }
    std::size_t ringCount() const { return rings.size(); }
    std::size_t slashCount() const { return slashes.size(); }
    std::size_t floatingTextCount() const { return floatingTexts.size(); }
    std::size_t beamCount() const { return beams.size(); }
    std::size_t skillCastEventCount() const { return skillCastEvents; }
    std::size_t activeEffectCount() const {
        return particles.size() + rings.size() + slashes.size() +
               beams.size() + floatingTexts.size();
    }

private:
    struct Particle {
        sf::Vector2f position;
        sf::Vector2f velocity;
        sf::Color color;
        float radius = 2.f;
        float life = 0.f;
        float maxLife = 0.f;
        float drag = 0.f;
        float gravity = 0.f;
        bool ground = false;
    };

    struct Ring {
        sf::Vector2f position;
        sf::Color color;
        float startRadius = 0.f;
        float endRadius = 0.f;
        float life = 0.f;
        float maxLife = 0.f;
        float thickness = 2.f;
        bool ground = false;
    };

    struct Slash {
        sf::Vector2f position;
        sf::Vector2f direction;
        sf::Color color;
        float scale = 1.f;
        float life = 0.f;
        float maxLife = 0.f;
    };

    struct Beam {
        sf::Vector2f start;
        sf::Vector2f end;
        sf::Color color;
        float life = 0.f;
        float maxLife = 0.f;
    };

    struct FloatingText {
        sf::Vector2f position;
        std::string value;
        sf::Color color;
        unsigned size = 18;
        float life = 0.f;
        float maxLife = 0.f;
    };

    sf::Font font;
    bool fontLoaded = false;
    std::vector<Particle> particles;
    std::vector<Ring> rings;
    std::vector<Slash> slashes;
    std::vector<Beam> beams;
    std::vector<FloatingText> floatingTexts;
    std::mt19937 random{0xE7E4A1u};
    float pendingShakeStrength = 0.f;
    std::size_t skillCastEvents = 0;

    float randomFloat(float minValue, float maxValue);
    void addBurst(sf::Vector2f position, sf::Color color, int count,
                  float minSpeed, float maxSpeed, float minRadius,
                  float maxRadius, float life, float gravity = 0.f);
    void pushParticle(Particle particle);
    void pushRing(Ring ring);
    void pushSlash(Slash slash);
    void pushBeam(Beam beam);
    void pushFloatingText(FloatingText text);
    void enforceEffectInstanceLimit();
};
