#pragma once

#include "EnemyConfig.h"
#include "Monster.h"

#include <array>

class Player;

class Boss final : public Monster {
public:
    enum class Phase {
        Phase1,
        Phase2,
        Enraged
    };

    enum class BeamState {
        Inactive,
        Warning,
        Active,
        Cooldown
    };

    static constexpr float PHASE_TWO_HEALTH_RATIO = 0.50f;
    static constexpr float ENRAGE_HEALTH_RATIO = 0.25f;
    static constexpr float BEAM_WARNING_DURATION = 0.80f;
    static constexpr float BEAM_ACTIVE_DURATION = 0.35f;
    static constexpr float BEAM_COOLDOWN_DURATION = 4.00f;
    static constexpr float BEAM_DAMAGE = 22.f;
    static constexpr float ENRAGED_SPEED_MULTIPLIER = 1.45f;
    static constexpr float ENRAGED_DAMAGE_TAKEN_MULTIPLIER = 0.65f;

    Boss(float x, float y, int waveNumber);
    bool isBoss() const override { return true; }
    void takeDamage(float damage) override;
    void update(GameContext& context) override;
    void draw(sf::RenderWindow& window) override;

    Phase getPhase() const { return phase; }
    bool isEnraged() const { return phase == Phase::Enraged; }
    static constexpr float getEnrageThresholdRatio() {
        return ENRAGE_HEALTH_RATIO;
    }
    BeamState getBeamState() const { return beamState; }
    float getBeamTimer() const { return beamTimer; }
    int getPhaseTwoActivationCount() const {
        return phaseTwoActivationCount;
    }
    int getEnrageActivationCount() const { return enrageActivationCount; }
    int getBeamVolleyCount() const { return beamVolleyCount; }

private:
    static constexpr std::size_t BEAM_COUNT = 8;
    static constexpr float BEAM_LENGTH = 900.f;
    static constexpr float BEAM_WARNING_WIDTH = 10.f;
    static constexpr float BEAM_ACTIVE_WIDTH = 18.f;

    sf::CircleShape aura;
    std::array<sf::RectangleShape, BEAM_COUNT> beamShapes;
    Phase phase = Phase::Phase1;
    BeamState beamState = BeamState::Inactive;
    float baseMoveSpeed = 0.f;
    float beamTimer = 0.f;
    bool playerHitThisVolley = false;
    int phaseTwoActivationCount = 0;
    int enrageActivationCount = 0;
    int beamVolleyCount = 0;

    Boss(float x, float y, const EnemyConfig::Stats& stats);
    void activatePhaseTwo();
    void activateEnraged();
    void startBeamWarning();
    void updateBeam(GameContext& context);
    Player* findPlayer(const GameContext& context) const;
    bool beamHitsPlayer(const Player& player) const;
    void configureBeamVisuals();
};
