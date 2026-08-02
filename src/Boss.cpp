#include "Boss.h"

#include "GameContext.h"
#include "Player.h"
#include "Effects.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr float PI = 3.14159265358979323846f;
}

Boss::Boss(float x, float y, int waveNumber)
    : Boss(x, y, EnemyConfig::bossStats(waveNumber)) {}

Boss::Boss(float x, float y, const EnemyConfig::Stats& stats)
    : Monster(x, y,
              stats.maxHealth,
              stats.maxHealth,
              stats.attackRange,
              stats.cooldown,
              stats.speed,
              static_cast<float>(stats.effectiveDamage)),
      aura(31.f),
      baseMoveSpeed(stats.speed) {
    collisionSize = {44.f, 44.f};
    hurtBoxSize = {58.f, 58.f};
    visualSize = {58.f, 58.f};
    eliteVisual = true;
    presentationTint = sf::Color(130, 45, 185);
    monsterShape.setSize({58.f, 58.f});
    monsterShape.setOrigin({29.f, 29.f});
    monsterShape.setFillColor(sf::Color(130, 45, 185));

    aura.setRadius(54.f);
    aura.setOrigin({54.f, 54.f});
    aura.setFillColor(sf::Color(220, 80, 255, 45));
    aura.setOutlineColor(sf::Color(235, 150, 255));
    aura.setOutlineThickness(2.f);
    configureBeamVisuals();
    setGoldReward(150);
}

void Boss::configureBeamVisuals() {
    for (std::size_t i = 0; i < beamShapes.size(); ++i) {
        const float angle = 360.f * static_cast<float>(i) /
                            static_cast<float>(beamShapes.size());
        beamShapes[i].setRotation(sf::degrees(angle));
    }
}

void Boss::activatePhaseTwo() {
    if (phase != Phase::Phase1 || isDead()) return;
    phase = Phase::Phase2;
    ++phaseTwoActivationCount;
    aura.setOutlineColor(sf::Color(255, 185, 65));
    startBeamWarning();
}

void Boss::activateEnraged() {
    if (isEnraged() || isDead()) return;
    if (phase == Phase::Phase1) activatePhaseTwo();
    phase = Phase::Enraged;
    ++enrageActivationCount;
    moveSpeed = baseMoveSpeed * ENRAGED_SPEED_MULTIPLIER;
    monsterShape.setFillColor(sf::Color(220, 55, 110));
    presentationTint = sf::Color(220, 55, 110);
    aura.setFillColor(sf::Color(255, 45, 100, 62));
    aura.setOutlineColor(sf::Color::Red);
}

void Boss::takeDamage(float damage) {
    if (isEnraged()) {
        damage *= ENRAGED_DAMAGE_TAKEN_MULTIPLIER;
    }
    Monster::takeDamage(damage);
    if (isDead()) {
        beamState = BeamState::Inactive;
        beamTimer = 0.f;
        playerHitThisVolley = true;
        return;
    }

    const float healthRatio = getHealth() / getMaxHealth();
    if (healthRatio <= ENRAGE_HEALTH_RATIO) {
        activateEnraged();
    } else if (healthRatio <= PHASE_TWO_HEALTH_RATIO) {
        activatePhaseTwo();
    }
}

void Boss::startBeamWarning() {
    beamState = BeamState::Warning;
    beamTimer = BEAM_WARNING_DURATION;
    playerHitThisVolley = false;
    ++beamVolleyCount;
}

Player* Boss::findPlayer(const GameContext& context) const {
    for (Entity* target : context.players) {
        if (auto* player = dynamic_cast<Player*>(target);
            player && !player->isDead()) {
            return player;
        }
    }
    return nullptr;
}

bool Boss::beamHitsPlayer(const Player& player) const {
    const sf::Vector2f offset = player.getPosition() - getPosition();
    const sf::FloatRect collision = player.getCollisionBox();
    const float playerRadius =
        std::min(collision.size.x, collision.size.y) * 0.5f;
    for (std::size_t i = 0; i < BEAM_COUNT; ++i) {
        const float angle = 2.f * PI * static_cast<float>(i) /
                            static_cast<float>(BEAM_COUNT);
        const sf::Vector2f direction{std::cos(angle), std::sin(angle)};
        const float projection = offset.x * direction.x +
                                 offset.y * direction.y;
        if (projection < 0.f || projection > BEAM_LENGTH) continue;
        const float perpendicular = std::abs(
            offset.x * direction.y - offset.y * direction.x);
        if (perpendicular <= BEAM_ACTIVE_WIDTH * 0.5f + playerRadius) {
            return true;
        }
    }
    return false;
}

void Boss::updateBeam(GameContext& context) {
    if (beamState == BeamState::Inactive || isDead()) return;

    float remaining = std::max(0.f, context.deltaTime);
    while (remaining > 0.f) {
        if (beamState == BeamState::Active && !playerHitThisVolley) {
            if (Player* player = findPlayer(context);
                player && beamHitsPlayer(*player)) {
                const float before = player->getHealth();
                player->takeDamage(BEAM_DAMAGE);
                if (context.effects) {
                    const float dealt = std::max(
                        0.f, before - player->getHealth());
                    context.effects->spawnImpact(
                        player->getPosition(), ImpactStyle::Enemy);
                    if (dealt > 0.f) {
                        context.effects->spawnFloatingNumber(
                            player->getPosition(), dealt, false, true);
                    }
                    context.effects->requestScreenShake(2.5f);
                }
                playerHitThisVolley = true;
            }
        }

        const float step = std::min(remaining, beamTimer);
        beamTimer -= step;
        remaining -= step;
        if (beamTimer > 0.f) break;

        switch (beamState) {
        case BeamState::Warning:
            beamState = BeamState::Active;
            beamTimer = BEAM_ACTIVE_DURATION;
            break;
        case BeamState::Active:
            beamState = BeamState::Cooldown;
            beamTimer = BEAM_COOLDOWN_DURATION;
            break;
        case BeamState::Cooldown:
            startBeamWarning();
            break;
        case BeamState::Inactive:
            return;
        }
    }
}

void Boss::update(GameContext& context) {
    if (context.paused) return;

    if (!isDead()) {
        const float healthRatio = getHealth() / getMaxHealth();
        if (healthRatio <= ENRAGE_HEALTH_RATIO) activateEnraged();
        else if (healthRatio <= PHASE_TWO_HEALTH_RATIO) activatePhaseTwo();
        updateBeam(context);
    }
    Monster::update(context);
}

void Boss::draw(sf::RenderWindow& window) {
    if (beamState == BeamState::Warning ||
        beamState == BeamState::Active) {
        const bool active = beamState == BeamState::Active;
        const float width = active ? BEAM_ACTIVE_WIDTH : BEAM_WARNING_WIDTH;
        const sf::Color color = active
            ? sf::Color(255, 45, 70, 190)
            : sf::Color(255, 210, 70, 100);
        for (auto& beam : beamShapes) {
            beam.setSize({BEAM_LENGTH, width});
            beam.setOrigin({0.f, width * 0.5f});
            beam.setPosition(position);
            beam.setFillColor(color);
            window.draw(beam);
        }
    }
    const float pulseSpeed = isEnraged() ? 4.4f : 3.2f;
    const float pulseAmount = isEnraged() ? 0.12f : 0.08f;
    const float auraPulse =
        1.f + std::sin(visualTime * pulseSpeed) * pulseAmount;
    aura.setScale({auraPulse, auraPulse});
    aura.setPosition(position);
    window.draw(aura);
    Monster::draw(window);
}
