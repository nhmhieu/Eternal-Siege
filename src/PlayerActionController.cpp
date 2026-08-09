#include "PlayerActionController.h"

void PlayerActionController::update(float dt, bool paused) {
    if (paused) return;
    dt = std::max(0.f, dt);
    dashCooldown = std::max(0.f, dashCooldown - dt);
    if (gesture == PrimaryGesture::PendingPrimary ||
        gesture == PrimaryGesture::ChargingHeavy) {
        heldTime += dt;
        if (heldTime >= PRIMARY_HOLD_THRESHOLD)
            gesture = PrimaryGesture::ChargingHeavy;
    }
}

void PlayerActionController::pressPrimary() {
    if (gesture != PrimaryGesture::Idle) return;
    gesture = PrimaryGesture::PendingPrimary;
    heldTime = 0.f;
}

void PlayerActionController::releasePrimary() {
    if (gesture == PrimaryGesture::PendingPrimary) {
        request = PlayerActionRequest::BasicAttack;
    } else if (gesture == PrimaryGesture::ChargingHeavy) {
        request = PlayerActionRequest::ReleaseHeavy;
    }
    gesture = PrimaryGesture::Idle;
}

void PlayerActionController::pressDash() {
    gesture = PrimaryGesture::Idle;
    heldTime = 0.f;
    request = PlayerActionRequest::None;
    if (dashCooldown <= 0.f) {
        request = PlayerActionRequest::Dash;
        dashCooldown = DASH_COOLDOWN;
    }
}

void PlayerActionController::cancel() {
    gesture = PrimaryGesture::Idle;
    heldTime = 0.f;
    request = PlayerActionRequest::None;
}

void PlayerActionController::reset() {
    cancel();
    dashCooldown = 0.f;
}

PlayerActionRequest PlayerActionController::consumeRequest() {
    const auto result = request;
    request = PlayerActionRequest::None;
    return result;
}

float PlayerActionController::getChargeRatio() const {
    return std::clamp((heldTime - PRIMARY_HOLD_THRESHOLD) /
        (HEAVY_MAX_HOLD_TIME - PRIMARY_HOLD_THRESHOLD), 0.f, 1.f);
}
