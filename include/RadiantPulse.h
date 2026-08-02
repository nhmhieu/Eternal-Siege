#pragma once

#include <vector>

class Ally;
class Player;

struct RadiantHealEvent {
    Ally* ally = nullptr;
    float amount = 0.f;
};

struct RadiantPulseResult {
    bool activated = false;
    std::vector<RadiantHealEvent> heals;
};

class RadiantPulse {
public:
    static constexpr float RADIUS = 210.f;
    static constexpr float HEAL_RATIO = 0.18f;
    static constexpr float COOLDOWN = 15.f;

    RadiantPulseResult tryActivate(
        Player& player, const std::vector<Ally*>& allies,
        bool combatActive);
    void update(float deltaTime, bool combatActive, bool paused);
    void reset();

    bool isReady() const { return cooldownRemaining <= 0.f; }
    float getCooldownRemaining() const { return cooldownRemaining; }

private:
    float cooldownRemaining = 0.f;
};
