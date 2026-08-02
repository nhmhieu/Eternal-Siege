#include "Ally.h"
#include "AssetLocator.h"
#include "AudioManager.h"
#include "Boss.h"
#include "BossHealthBar.h"
#include "CombatManager.h"
#include "Effects.h"
#include "GameContext.h"
#include "HUD.h"
#include "Map.h"
#include "Monster.h"
#include "Player.h"
#include "RadiantPulse.h"
#include "SpiritStaff.h"
#include "TextureManager.h"

#include <cassert>
#include <array>
#include <cmath>
#include <iostream>
#include <filesystem>
#include <memory>
#include <string_view>
#include <vector>

namespace {
constexpr float EPSILON = 0.001f;

void near(float actual, float expected) {
    if (std::abs(actual - expected) > EPSILON) {
        std::cerr << "near() failed: actual=" << actual
                  << " expected=" << expected << '\n';
    }
    assert(std::abs(actual - expected) <= EPSILON);
}

void loadTextures(TextureManager& textures) {
    assert(textures.loadTexture(
        std::string(Player::TEXTURE_KEY), "assets/images/PlayerMage.png"));
    assert(textures.loadTexture(
        std::string(SpiritStaff::TEXTURE_KEY),
        "assets/images/SpiritStaff.png"));
    for (const char* name : {"Damian", "Evangeline", "Junior", "Lucas"}) {
        textures.loadTexture(
            name, std::string("assets/images/") + name + ".png");
    }
}

void testRadiantPulse(TextureManager& textures) {
    Player player(textures);
    player.setPosition({300.f, 300.f});
    const float playerHealth = player.getHealth();

    Ally junior(330.f, 300.f, textures, AllyType::Junior);
    Ally damian(360.f, 300.f, textures, AllyType::Damian);
    Ally full(390.f, 300.f, textures, AllyType::Lucas);
    Ally outside(511.f, 300.f, textures, AllyType::Evangeline);
    Ally dead(320.f, 300.f, textures, AllyType::Lucas);
    junior.takeDamage(200.f);
    damian.takeDamage(200.f);
    outside.takeDamage(200.f);
    dead.takeDamage(10000.f);

    const float juniorBefore = junior.getHealth();
    const float damianBefore = damian.getHealth();
    const float outsideBefore = outside.getHealth();
    RadiantPulse pulse;
    const auto result = pulse.tryActivate(
        player, {&junior, &damian, &full, &outside, &dead}, true);
    assert(result.activated && result.heals.size() == 2);
    near(junior.getHealth() - juniorBefore,
         std::round(junior.getMaxHealth() * 0.18f));
    near(damian.getHealth() - damianBefore,
         std::round(damian.getMaxHealth() * 0.18f));
    near(outside.getHealth(), outsideBefore);
    near(full.getHealth(), full.getMaxHealth());
    assert(dead.isDead());
    near(dead.getHealth(), 0.f);
    near(player.getHealth(), playerHealth);
    near(pulse.getCooldownRemaining(), 15.f);

    pulse.update(2.f, true, true);
    near(pulse.getCooldownRemaining(), 15.f);
    pulse.update(2.f, false, false);
    near(pulse.getCooldownRemaining(), 15.f);
    pulse.update(14.9f, true, false);
    assert(!pulse.isReady());
    pulse.update(0.11f, true, false);
    assert(pulse.isReady());

    RadiantPulse noTarget;
    const auto none = noTarget.tryActivate(player, {&full, &dead}, true);
    assert(!none.activated && none.heals.empty() && noTarget.isReady());
    const auto inactive = noTarget.tryActivate(player, {&junior}, false);
    assert(!inactive.activated && noTarget.isReady());

    RadiantPulse clampPulse;
    Ally clampTarget(320.f, 300.f, textures, AllyType::Damian);
    clampTarget.takeDamage(5.f);
    const auto clamped = clampPulse.tryActivate(
        player, {&clampTarget}, true);
    assert(clamped.activated && clamped.heals.size() == 1);
    near(clamped.heals[0].amount, 5.f);
    near(clampTarget.getHealth(), clampTarget.getMaxHealth());
}

void testEffectSafety() {
    Effects effects;
    for (int i = 0; i < 1000; ++i) {
        effects.spawnFootstep({100.f, 100.f}, FootstepStyle::Boss);
        effects.spawnFloatingNumber({100.f, 100.f}, 10.f, false);
        effects.spawnPortal({100.f, 100.f}, true);
    }
    assert(effects.particleCount() <= Effects::MAX_PARTICLES);
    assert(effects.floatingTextCount() <= Effects::MAX_FLOATING_TEXTS);
    assert(effects.activeEffectCount() <=
           Effects::MAX_PARTICLES + Effects::MAX_EFFECT_INSTANCES +
               Effects::MAX_FLOATING_TEXTS);

    effects.clear();
    effects.spawnFloatingNumber({20.f, 20.f}, 12.f, false);
    const std::size_t pausedCount = effects.floatingTextCount();
    effects.update(5.f, true);
    assert(effects.floatingTextCount() == pausedCount);
    effects.update(5.f, false);
    assert(effects.floatingTextCount() == 0);
}

void testAudioPolicy(TextureManager& textures) {
    assert(AudioManager::MAX_VOICES >= 24);
    assert(AudioManager::MAX_VOICES <= 32);
    const std::array<std::string_view, 11> expectedKeys{{
        "ui_click", "spirit_bolt", "melee_hit", "bow_shot",
        "enemy_death", "radiant_pulse", "ally_skill", "wave_start",
        "boss_spawn", "victory", "defeat"}};
    const auto& assets = AudioManager::soundAssets();
    assert(assets.size() == expectedKeys.size());
    for (std::size_t index = 0; index < assets.size(); ++index) {
        assert(assets[index].key == expectedKeys[index]);
        const auto path = AssetLocator::find(
            std::string(assets[index].relativePath));
        assert(path && std::filesystem::file_size(*path) > 44);
    }

    AudioManager audio(false);
    assert(!audio.isMuted());
    assert(audio.handleMuteKeyPressed());
    assert(audio.isMuted());
    assert(!audio.handleMuteKeyPressed());
    assert(audio.isMuted());
    assert(!audio.playSound("ui_click"));
    assert(audio.acceptedPlayCount("ui_click") == 0);
    audio.handleMuteKeyReleased();
    assert(audio.handleMuteKeyPressed());
    assert(!audio.isMuted());
    audio.handleMuteKeyReleased();

    assert(audio.playSound("ui_click"));
    assert(!audio.playSound("ui_click"));
    assert(audio.acceptedPlayCount("ui_click") == 1);
    audio.update(0.06f);
    assert(audio.playSound("ui_click"));
    assert(audio.acceptedPlayCount("ui_click") == 2);

    for (const std::string_view key : {
             "victory", "defeat", "wave_start", "boss_spawn"}) {
        assert(audio.playSound(key));
        assert(!audio.playSound(key));
        assert(audio.acceptedPlayCount(key) == 1);
    }

    Player cooldownProbe(textures);
    cooldownProbe.setAttackCooldown(0.55f);
    const float gameplayCooldown = cooldownProbe.getAttackCooldown();
    audio.update(10.f);
    near(cooldownProbe.getAttackCooldown(), gameplayCooldown);

    assert(!audio.playSfx(
        "missing_test", "assets/audio/sfx/intentionally_missing.wav"));
    assert(audio.activeVoiceCount() == 0);
}

void testBossBarSnapshot() {
    BossHealthBar bar;
    auto boss = std::make_unique<Boss>(300.f, 300.f, 4);
    bar.update(boss.get(), 0.1f, false);
    assert(bar.isVisible());
    near(bar.getDisplayedHealth(), boss->getHealth());
    boss->takeDamage(100000.f);
    bar.update(boss.get(), 0.1f, false);
    assert(!bar.isVisible());
    boss.reset();
    bar.update(nullptr, 0.1f, false);
    assert(!bar.isVisible());
}

void testResponsiveLayouts() {
    for (const sf::Vector2f view : {
            sf::Vector2f{1280.f, 720.f}, sf::Vector2f{960.f, 540.f},
            sf::Vector2f{1920.f, 1080.f}}) {
        const sf::Vector2f bossSize = BossHealthBar::calculateBarSize(view);
        assert(bossSize.x >= 360.f && bossSize.x <= 560.f);
        assert(bossSize.y >= 16.f && bossSize.y <= 21.f);
        const HUDLayout layout = HUD::calculateLayout(view);
        for (const sf::FloatRect rect : {
                layout.playerCard, layout.allyCard, layout.upgradeCard,
                layout.radiantCard, layout.controlsCard}) {
            assert(rect.size.x > 0.f && rect.size.y > 0.f);
            assert(rect.position.x >= 0.f && rect.position.y >= 0.f);
            assert(rect.position.x + rect.size.x <=
                   std::max(view.x, 640.f) + EPSILON);
            assert(rect.position.y + rect.size.y <=
                   std::max(view.y, 480.f) + EPSILON);
        }
    }
}

void testSpiritBoltAndVisualIsolation(TextureManager& textures) {
    assert(Player::TEXTURE_KEY == "PlayerMage");
    assert(Player::TEXTURE_KEY != "Ash");
    Player player(textures);
    player.setPosition({300.f, 300.f});
    const sf::Vector2f logicBefore = player.getPosition();
    const sf::FloatRect collisionBefore = player.getCollisionBox();
    const sf::FloatRect hurtBefore = player.getHurtBox();
    const sf::Vector2f visualSize = player.getVisualSize();
    assert(visualSize.y >= 75.f && visualSize.y <= 90.f);
    assert(visualSize.x > 30.f && visualSize.x < 70.f);
    auto staffWeapon = std::make_unique<SpiritStaff>(textures);
    SpiritStaff* staff = staffWeapon.get();
    assert(SpiritStaff::TEXTURE_KEY == "SpiritStaff");
    assert(staff->usesSpriteTexture());
    const sf::Vector2f staffDisplaySize = staff->getDisplaySize();
    assert(staffDisplaySize.y >= 76.f && staffDisplaySize.y <= 80.f);
    player.setCurrentWeapon(std::move(staffWeapon));

    for (const sf::Vector2f aim : {
             sf::Vector2f{1.f, 0.f}, sf::Vector2f{0.f, 1.f},
             sf::Vector2f{-1.f, 0.f}, sf::Vector2f{0.f, -1.f}}) {
        player.setAimDirection(aim);
        const sf::Vector2f anchor = player.getSpiritStaffAnchor();
        const sf::Vector2f renderedGrip =
            staff->getGripPosition(anchor, aim);
        near(renderedGrip.x, anchor.x);
        near(renderedGrip.y, anchor.y);
        const sf::Vector2f tip = staff->getTipPosition(anchor, aim);
        const float gripToTip = std::hypot(
            tip.x - renderedGrip.x, tip.y - renderedGrip.y);
        assert(gripToTip >= 54.f && gripToTip <= 56.f);
        near(player.getPosition().x, logicBefore.x);
        near(player.getPosition().y, logicBefore.y);
        near(player.getCollisionBox().size.x, collisionBefore.size.x);
        near(player.getCollisionBox().size.y, collisionBefore.size.y);
        near(player.getHurtBox().size.x, hurtBefore.size.x);
        near(player.getHurtBox().size.y, hurtBefore.size.y);
    }

    player.setAimDirection({1.f, 0.f});
    const sf::Vector2f staffAnchor = player.getSpiritStaffAnchor();
    const sf::Vector2f staffTip = player.getSpiritStaffTip();
    const float anchorDistance = std::hypot(
        staffAnchor.x - logicBefore.x, staffAnchor.y - logicBefore.y);
    assert(anchorDistance >= 10.f && anchorDistance <= 32.f);
    assert(std::hypot(staffTip.x - staffAnchor.x,
                      staffTip.y - staffAnchor.y) >= 54.f);
    assert(std::hypot(staffTip.x - staffAnchor.x,
                      staffTip.y - staffAnchor.y) <= 56.f);
    player.setAttackPower(31.f);
    player.startAttacking();

    GameContext context;
    CombatManager combat;
    Effects effects;
    AudioManager audio(false);
    Map map(15, 15);
    context.combatManager = &combat;
    context.effects = &effects;
    context.audioManager = &audio;
    context.map = &map;
    context.deltaTime = 0.01f;
    player.getCurrentWeapon()->triggerAction(&player, context, combat);
    assert(context.projectiles.size() == 1);
    assert(audio.acceptedPlayCount("spirit_bolt") == 1);
    near(context.projectiles[0]->getPosition().x, staffTip.x);
    near(context.projectiles[0]->getPosition().y, staffTip.y);
    assert(context.projectiles[0]->getStyle() == ProjectileStyle::Spirit);
    near(context.projectiles[0]->getDamage(), 31.f);
    player.setAttackPower(99.f);
    near(context.projectiles[0]->getDamage(), 31.f);

    const sf::Vector2f impactPoint = staffTip + sf::Vector2f(10.f, 0.f);
    Ally friendly(
        impactPoint.x, impactPoint.y, textures, AllyType::Junior);
    Monster enemy(
        impactPoint.x, impactPoint.y,
        100.f, 100.f, 40.f, 1.f, 0.f, 5.f);
    const float friendlyBefore = friendly.getHealth();
    context.deltaTime = 0.02f;
    combat.processProjectiles(context, {&friendly, &enemy});
    near(friendly.getHealth(), friendlyBefore);
    near(enemy.getHealth(), 69.f);

    player.setIsAttacking(false);
    context.deltaTime = 0.2f;
    player.update(context);
    near(player.getPosition().x, logicBefore.x);
    near(player.getPosition().y, logicBefore.y);
    const sf::FloatRect collisionAfter = player.getCollisionBox();
    const sf::FloatRect hurtAfter = player.getHurtBox();
    near(collisionBefore.size.x, collisionAfter.size.x);
    near(collisionBefore.size.y, collisionAfter.size.y);
    near(collisionBefore.position.x, collisionAfter.position.x);
    near(collisionBefore.position.y, collisionAfter.position.y);
    near(hurtBefore.size.x, hurtAfter.size.x);
    near(hurtBefore.size.y, hurtAfter.size.y);
    near(hurtBefore.position.x, hurtAfter.position.x);
    near(hurtBefore.position.y, hurtAfter.position.y);

    player.setAttackPower(12.f);
    player.startAttacking();
    player.getCurrentWeapon()->setHasAttacked(false);
    player.getCurrentWeapon()->triggerAction(&player, context, combat);
    assert(context.projectiles.size() == 1);
    auto removedTarget = std::make_unique<Monster>(
        650.f, 300.f, 100.f, 100.f, 40.f, 1.f, 0.f, 5.f);
    removedTarget.reset();
    context.deltaTime = 0.05f;
    combat.processProjectiles(context, {});
    assert(context.projectiles.size() == 1);
}

void testSkillEffectSingleEvent(TextureManager& textures) {
    Ally junior(300.f, 300.f, textures, AllyType::Junior);
    Monster first(350.f, 300.f, 500.f, 500.f, 40.f, 1.f, 0.f, 5.f);
    Monster second(400.f, 300.f, 500.f, 500.f, 40.f, 1.f, 0.f, 5.f);
    Effects effects;
    AudioManager audio(false);
    GameContext context;
    context.effects = &effects;
    context.audioManager = &audio;
    context.enemies = {&first, &second};
    context.allySkillsEnabled = true;
    context.deltaTime = 0.01f;
    junior.update(context);
    assert(effects.skillCastEventCount() == 1);
    assert(audio.acceptedPlayCount("ally_skill") == 1);
    for (int frame = 0; frame < 20; ++frame) junior.update(context);
    assert(effects.skillCastEventCount() == 1);
    assert(audio.acceptedPlayCount("ally_skill") == 1);
}
}

int main() {
    TextureManager textures;
    loadTextures(textures);
    testRadiantPulse(textures);
    testEffectSafety();
    testAudioPolicy(textures);
    testBossBarSnapshot();
    testResponsiveLayouts();
    testSpiritBoltAndVisualIsolation(textures);
    testSkillEffectSingleEvent(textures);
    std::cout << "Support and presentation logic tests passed\n";
    return 0;
}
