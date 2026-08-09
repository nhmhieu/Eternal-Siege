#pragma once

#include <algorithm>

enum class PlayerActionRequest { None, BasicAttack, Dash, ReleaseHeavy };
enum class PrimaryGesture { Idle, PendingPrimary, ChargingHeavy };

class PlayerActionController {
public:
    static constexpr float PRIMARY_HOLD_THRESHOLD = 0.22f;
    static constexpr float HEAVY_MAX_HOLD_TIME = 1.10f;
    static constexpr float DASH_COOLDOWN = 0.80f;

    void update(float deltaTime, bool paused = false);
    void pressPrimary();
    void releasePrimary();
    void pressDash();
    void cancel();
    void reset();
    PlayerActionRequest consumeRequest();

    bool isCharging() const { return gesture == PrimaryGesture::ChargingHeavy; }
    bool hasActivePrimary() const { return gesture != PrimaryGesture::Idle; }
    PrimaryGesture getGesture() const { return gesture; }
    float getChargeRatio() const;
    float getDashCooldownRemaining() const { return dashCooldown; }
    bool isDashReady() const { return dashCooldown <= 0.f; }

private:
    PrimaryGesture gesture = PrimaryGesture::Idle;
    PlayerActionRequest request = PlayerActionRequest::None;
    float heldTime = 0.f;
    float dashCooldown = 0.f;
};
