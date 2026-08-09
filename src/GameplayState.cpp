#include "GameplayState.h"

#include "BalanceConfig.h"
#include "Boss.h"
#include "Constants.h"
#include "EntityCollision.h"
#include "EntityLifecycle.h"
#include "GameOverState.h"
#include "WinState.h"
#include "GameContext.h"
#include "AssetLocator.h"
#include "SpiritStaff.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>

using namespace GameConfig;

GameplayState::GameplayState(
    StateMachine& machine,
    sf::RenderWindow& gameWindow,
    TextureManager& textures,
    AudioManager& audio,
    GameProgress& gameProgress,
    LevelId levelId,
    const Map& setupMap,
    const std::vector<sf::Vector2i>& selectedAllyPositions)
    : stateMachine(machine),
      window(gameWindow),
      textureManager(textures),
      audioManager(audio),
      progress(gameProgress),
      selectedLevelId(levelId),
      map(setupMap),
      allyPositions(selectedAllyPositions) {

    if (!textureManager.loadTexture(
            std::string(Player::TEXTURE_KEY), "assets/images/PlayerMage.png")) {
        std::cerr << "Required player asset failed to load: "
                  << "assets/images/PlayerMage.png\n";
    }
    if (!textureManager.loadTexture(
            std::string(SpiritStaff::TEXTURE_KEY),
            "assets/images/SpiritStaff.png")) {
        std::cerr << "Required staff asset failed to load: "
                  << "assets/images/SpiritStaff.png\n";
    }
    for (const std::string name : {"Damian", "Evangeline", "Junior", "Lucas"}) {
        textureManager.loadTexture(
            name, "assets/images/characters/" + name + ".png");
    }
    textureManager.loadTexture(
        "MonsterNormal", "assets/images/monsters/NormalMonster.png");
    textureManager.loadTexture(
        "MonsterElite", "assets/images/monsters/EliteMonster.png");
    textureManager.loadTexture(
        "MonsterBoss", "assets/images/monsters/Boss.png");

    player = std::make_unique<Player>(textureManager);
    player->setAttackPower(
        static_cast<float>(BalanceConfig::PLAYER_DAMAGE));
    player->setCurrentWeapon(std::make_unique<SpiritStaff>(textureManager));
    player->setAttackCooldown(0.55f);
}

void GameplayState::onEnter() {
    actionController.reset();
    paused = false;
    context.paused = false;
    transitionGate.reset();
    effects.clear();
    radiantPulse.reset();
    bossHealthBar.load();
    bossEnrageNotice.resetForNewGame();
    bossEnrageNotice.load();
    deathEffectEntities.clear();
    shakeRemaining = 0.f;
    shakeElapsed = 0.f;
    shakeStrength = 0.f;
    lowHealthPulseTime = 0.f;
    hud.load();
    tutorialController.resetForNewGame();
    tutorialOverlay.load();
    intermissionController.resetForNewGame();
    intermissionOverlay.load();
    audioManager.playMusic("assets/audio/music/gameplay_theme.ogg");
    if (const auto path = AssetLocator::find("assets/fonts/Font.ttf");
        path && presentationFont.openFromFile(*path)) {
        bannerText.emplace(presentationFont, "", 28);
        bannerText->setFillColor(sf::Color(255, 220, 90));
    }

    const sf::Vector2i centerCell =
        map.nearestWalkable({map.getWidth() / 2, map.getHeight() / 2});
    player->setPosition(map.gridToWorld(centerCell));

    allies.clear();
    static constexpr std::array<AllyType, 4> types{{
        AllyType::Damian,
        AllyType::Evangeline,
        AllyType::Junior,
        AllyType::Lucas
    }};

    for (std::size_t i = 0; i < allyPositions.size() && i < types.size(); ++i) {
        const sf::Vector2f requestedPosition =
            map.gridToWorld(allyPositions[i]);
        const auto validPosition = map.findNearestValidPosition(
            requestedPosition, {20.f, 20.f});
        if (!validPosition) {
            continue;
        }
        allies.push_back(createAlly(
            types[i], *validPosition, textureManager));
    }

    context.combatManager = &combatManager;
    context.map = &map;
    context.effects = &effects;
    context.audioManager = &audioManager;
    rebuildContext();
}

void GameplayState::onExit() {
    actionController.cancel();
}

void GameplayState::rebuildContext() {
    context.allEntity.clear();
    context.players.clear();
    context.enemies.clear();
    context.monsters.clear();
    context.allies.clear();

    context.allEntity.push_back(player.get());
    if (!player->isDead()) {
        context.players.push_back(player.get());
    }

    for (auto& ally : allies) {
        context.allEntity.push_back(ally.get());
        if (!ally->isDead()) {
            context.players.push_back(ally.get());
            context.allies.push_back(ally.get());
        }
    }

    for (auto& monster : monsters) {
        context.allEntity.push_back(monster.get());
        if (!monster->isDead()) {
            context.enemies.push_back(monster.get());
            context.monsters.push_back(monster.get());
        }
    }
}

void GameplayState::collectRewardsAndRemoveDead() {
    for (auto& monster : monsters) {
        if (monster->isDead()) {
            upgradeManager.addGold(monster->claimGoldReward());
        }
    }

    std::vector<Entity*> readyToRemove;
    for (const auto& monster : monsters) {
        if (monster->isReadyToBeDelete()) {
            readyToRemove.push_back(monster.get());
        }
    }
    for (const auto& ally : allies) {
        if (ally->isReadyToBeDelete()) {
            readyToRemove.push_back(ally.get());
        }
    }

    EntityLifecycle::invalidateReferencesTo(
        readyToRemove, context, combatManager, true);
    for (const Entity* entity : readyToRemove) {
        deathEffectEntities.erase(entity);
    }

    monsters.erase(
        std::remove_if(monsters.begin(), monsters.end(),
            [](const std::unique_ptr<Monster>& monster) {
                return monster->isReadyToBeDelete();
            }),
        monsters.end());

    allies.erase(
        std::remove_if(allies.begin(), allies.end(),
            [](const std::unique_ptr<Ally>& ally) {
                return ally->isReadyToBeDelete();
            }),
        allies.end());
}

void GameplayState::startNextWave() {
    if (waveManager.isIntermission()) {
        waveManager.startNextWave();
        if (waveManager.isWaveActive()) {
            upgradeManager.clearUndoHistory();
            intermissionController.sync(false, waveManager.getCurrentWave());
            showWaveBanner(waveManager.getCurrentWave());
            audioManager.playSound("wave_start");
        }
    }
}

void GameplayState::showWaveBanner(int wave) {
    static constexpr std::array<const char*, 4> titles{{
        "Wave 1 - The First Breach",
        "Wave 2 - Gathering Darkness",
        "Wave 3 - Elite Assault",
        "Wave 4 - The Final Siege"
    }};
    if (wave >= 1 && wave <= static_cast<int>(titles.size())) {
        showBanner(titles[static_cast<std::size_t>(wave - 1)]);
    }
}

void GameplayState::showBanner(const std::string& text, float duration) {
    if (!bannerText) return;
    bannerText->setString(text);
    const sf::FloatRect bounds = bannerText->getLocalBounds();
    bannerText->setOrigin({bounds.position.x + bounds.size.x / 2.f,
                           bounds.position.y + bounds.size.y / 2.f});
    bannerText->setPosition({360.f, 96.f});
    bannerTimer = duration;
}

void GameplayState::handleEvent(const sf::Event& event) {
    if (event.is<sf::Event::FocusLost>()) {
        actionController.cancel();
        player->setHeavyCharging(false);
        return;
    }
    if (const auto* released = event.getIf<sf::Event::KeyReleased>()) {
        if (released->code == sf::Keyboard::Key::Enter) {
            tutorialController.handleKeyReleased(TutorialKey::Enter);
            intermissionController.handleEnterReleased();
        } else if (released->code == sf::Keyboard::Key::R) {
            tutorialController.handleKeyReleased(TutorialKey::Help);
        }
    }

    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Enter ||
            key->code == sf::Keyboard::Key::R) {
            const TutorialKey tutorialKey =
                key->code == sf::Keyboard::Key::Enter
                    ? TutorialKey::Enter : TutorialKey::Help;
            const TutorialTransition transition =
                tutorialController.handleKeyPressed(tutorialKey);
            if (transition == TutorialTransition::InitialClosed) {
                showWaveBanner(waveManager.getCurrentWave());
                audioManager.playSound("wave_start");
            }
            if (transition != TutorialTransition::Ignored) {
                if (tutorialController.blocksGameplayInput()) {
                    actionController.cancel();
                    player->setHeavyCharging(false);
                }
                return;
            }
        }

        if (tutorialController.blocksGameplayInput()) return;

        if (key->code == sf::Keyboard::Key::P) {
            paused = !paused;
            context.paused = paused;
            if (paused) {
                actionController.cancel();
                player->setHeavyCharging(false);
                audioManager.pauseMusic();
            }
            else audioManager.resumeMusic();
            return;
        }

        if (paused) {
            return;
        }

        if (key->code == sf::Keyboard::Key::LShift ||
            key->code == sf::Keyboard::Key::RShift) {
            actionController.pressDash();
            player->setHeavyCharging(false);
            return;
        }

        if (key->code == sf::Keyboard::Key::Enter) {
            if (waveManager.isIntermission()) {
                const IntermissionAction action =
                    intermissionController.handleEnter(
                        upgradeManager.getPendingPurchaseCount());
                if (action == IntermissionAction::BeginNextWave) {
                    startNextWave();
                }
            } else {
                startNextWave();
            }
            return;
        }

        if (waveManager.isIntermission()) {
            const auto tryPurchase = [this](UpgradeType type) {
                const UpgradePreview before = upgradeManager.preview(type);
                const bool purchased =
                    upgradeManager.purchase(type, *player, allies);
                intermissionController.notifyPurchase(
                    type, purchased, before.maxLevel);
                if (purchased) audioManager.playSound("upgrade");
            };
            if (key->code == sf::Keyboard::Key::Num1) {
                tryPurchase(UpgradeType::Damage);
            } else if (key->code == sf::Keyboard::Key::Num2) {
                tryPurchase(UpgradeType::Vitality);
            } else if (key->code == sf::Keyboard::Key::Num3) {
                tryPurchase(UpgradeType::FireRate);
            } else if (key->code == sf::Keyboard::Key::Backspace) {
                const bool undone =
                    upgradeManager.undoLastPurchase(*player, allies);
                intermissionController.notifyUndo(undone);
            }
        }

        if (key->code == sf::Keyboard::Key::Q &&
            !actionController.hasActivePrimary()) {
            const RadiantPulseResult result = radiantPulse.tryActivate(
                *player, context.allies, waveManager.isWaveActive());
            if (result.activated) {
                hud.notifyRadiantCast();
                effects.spawnRadiantPulse(
                    player->getPosition(), RadiantPulse::RADIUS);
                for (const RadiantHealEvent& heal : result.heals) {
                    effects.spawnHealLink(
                        player->getPosition(), heal.ally->getPosition());
                    effects.spawnFloatingNumber(
                        heal.ally->getPosition(), heal.amount, true);
                }
                audioManager.playSound("radiant_pulse");
            } else if (waveManager.isWaveActive()) {
                showBanner("No injured ally in range", 1.2f);
            }
            return;
        }
    }

    if (tutorialController.blocksGameplayInput() || paused ||
        waveManager.isIntermission()) {
        return;
    }

    if (const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouse->button == sf::Mouse::Button::Right) {
            actionController.pressDash();
            player->setHeavyCharging(false);
            return;
        }
        if (mouse->button == sf::Mouse::Button::Left)
            actionController.pressPrimary();
    }
    if (const auto* mouse = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (mouse->button == sf::Mouse::Button::Left)
            actionController.releasePrimary();
    }
}

void GameplayState::update(float dt) {
    finishTutorialInputGuardIfReleased();
    if (tutorialController.blocksGameplayUpdate() ||
        paused || transitionGate.hasRequest()) {
        return;
    }

    // Tranh mot frame qua lon khi keo/cat cua so lam entity "teleport".
    context.deltaTime = std::min(dt, 0.05f);
    context.paused = false;
    bannerTimer = std::max(0.f, bannerTimer - context.deltaTime);
    context.combatManager = &combatManager;
    context.map = &map;
    context.effects = &effects;
    context.audioManager = &audioManager;
    context.allySkillsEnabled = true;

    const sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
    aimWorldPosition = window.mapPixelToCoords(mousePixel);
    player->setAimDirection(aimWorldPosition - player->getPosition());
    if (waveManager.isIntermission()) actionController.cancel();
    else actionController.update(context.deltaTime, false);
    player->setHeavyCharging(actionController.isCharging(),
                             actionController.getChargeRatio());
    const float releaseRatio = actionController.getChargeRatio();
    switch (actionController.consumeRequest()) {
    case PlayerActionRequest::BasicAttack:
        if (player->canAttack()) player->setIsAttacking(true);
        break;
    case PlayerActionRequest::Dash:
        player->beginDash(player->getDirection(),
            aimWorldPosition - player->getPosition(), &effects);
        break;
    case PlayerActionRequest::ReleaseHeavy:
        player->releaseHeavy(context, releaseRatio);
        break;
    case PlayerActionRequest::None: break;
    }
    radiantPulse.update(
        context.deltaTime, waveManager.isWaveActive(), false);
    lowHealthPulseTime += context.deltaTime;
    hud.updateAnimation(context.deltaTime, false);
    intermissionController.update(context.deltaTime, false);
    bossEnrageNotice.update(context.deltaTime, false);
    shakeElapsed += context.deltaTime;
    shakeRemaining = std::max(0.f, shakeRemaining - context.deltaTime);

    rebuildContext();

    const bool noMonstersAlive = std::none_of(
        monsters.begin(),
        monsters.end(),
        [](const std::unique_ptr<Monster>& monster) {
            return monster && !monster->isDead();
        });

    if (auto spawned = waveManager.update(
            context.deltaTime, map, noMonstersAlive)) {
        if (spawned->isBoss()) {
            spawned->setPresentationTexture(
                textureManager.findTexture("MonsterBoss"),
                textureManager.getVisibleBounds("MonsterBoss"), 122.f);
            showBanner("THE ABYSSAL LORD HAS ENTERED THE BATTLEFIELD", 3.f);
            audioManager.playMusic("assets/audio/music/boss_theme.ogg");
            audioManager.playSound("boss_spawn");
        } else if (spawned->isElite()) {
            spawned->setPresentationTexture(
                textureManager.findTexture("MonsterElite"),
                textureManager.getVisibleBounds("MonsterElite"), 72.f,
                sf::Color(255, 210, 145));
        } else {
            spawned->setPresentationTexture(
                textureManager.findTexture("MonsterNormal"),
                textureManager.getVisibleBounds("MonsterNormal"), 58.f);
        }
        effects.spawnPortal(spawned->getPosition(), spawned->isBoss());
        monsters.push_back(std::move(spawned));
        rebuildContext();
    }
    intermissionController.sync(
        waveManager.isIntermission(), waveManager.getCurrentWave());

    // allEntity la snapshot observer cua frame hien tai.
    for (Entity* entity : context.allEntity) {
        if (entity) entity->update(context);
    }

    EntityCollision::separateLivingEntities(
        map, context.monsters, context.allies);

    combatManager.processProjectiles(context, context.allEntity);

    const Boss* bossAfterCombat = findLivingBoss();
    bossEnrageNotice.observe(
        bossAfterCombat != nullptr,
        bossAfterCombat && bossAfterCombat->isEnraged());

    std::vector<Entity*> logicallyDead;
    for (Entity* entity : context.allEntity) {
        if (entity && entity->isDead()) {
            logicallyDead.push_back(entity);
            if (const auto* monster = dynamic_cast<const Monster*>(entity);
                monster && deathEffectEntities.insert(entity).second) {
                effects.spawnDeath(entity->getPosition(), monster->isBoss());
                audioManager.playSound("enemy_death");
            }
        }
    }
    EntityLifecycle::invalidateReferencesTo(
        logicallyDead, context, combatManager, false);

    effects.update(context.deltaTime);
    if (const float requestedShake = effects.consumeScreenShakeRequest();
        requestedShake > 0.f) {
        shakeStrength = requestedShake;
        shakeRemaining = 0.16f;
        shakeElapsed = 0.f;
    }

    collectRewardsAndRemoveDead();
    rebuildContext();

    hud.update(*player, waveManager, upgradeManager,
               static_cast<int>(monsters.size()), radiantPulse, allies);
    hud.setSoundMuted(audioManager.isMuted());
    bossHealthBar.update(
        findLivingBoss(), context.deltaTime, false);

    if (player->isDead()) {
        actionController.cancel();
        if (transitionGate.request(GameplayEndState::GameOver)) {
            stateMachine.changeState(std::make_unique<GameOverState>(
                stateMachine, window, textureManager, audioManager, progress,
                selectedLevelId));
        }
        return;
    }

    if (waveManager.isGameCompleted() && monsters.empty()) {
        if (transitionGate.request(GameplayEndState::Victory)) {
            const auto& definition = LEVEL_DEFINITIONS[
                static_cast<std::size_t>(selectedLevelId)];
            const RunResult result = progress.grantVictory(
                selectedLevelId, definition.clearReward);
            stateMachine.changeState(std::make_unique<WinState>(
                stateMachine, window, textureManager, audioManager, progress,
                result));
        }
    }
}

void GameplayState::render(sf::RenderWindow& target) {
    const sf::View baseView = target.getView();
    if (shakeRemaining > 0.f) {
        sf::View shakenView = baseView;
        shakenView.move({
            std::sin(shakeElapsed * 95.f) * shakeStrength,
            std::cos(shakeElapsed * 121.f) * shakeStrength * 0.7f});
        target.setView(shakenView);
    }

    map.draw(target);

    player->drawShadow(target);
    for (const auto& ally : allies) ally->drawShadow(target);
    for (const auto& monster : monsters) monster->drawShadow(target);
    effects.drawGround(target);

    player->draw(target);

    for (const auto& ally : allies) {
        ally->draw(target);
    }

    for (const auto& monster : monsters) {
        monster->draw(target);
    }

    player->drawHealthBar(target);
    for (const auto& ally : allies) ally->drawHealthBar(target);
    for (const auto& monster : monsters) monster->drawHealthBar(target);

    for (const auto& projectile : context.projectiles) {
        projectile->draw(target);
    }

    effects.drawCombat(target);
    effects.drawFloating(target);

    target.setView(baseView);
    bossHealthBar.draw(target);
    hud.draw(target);
    if (bannerText && bannerTimer > 0.f) target.draw(*bannerText);
    bossEnrageNotice.draw(target);
    drawAimingReticle(target);
    drawLowHealthFeedback(target);
    intermissionOverlay.draw(
        target, intermissionController, upgradeManager);
    if (paused && !tutorialController.isVisible()) drawPauseOverlay(target);
    tutorialOverlay.draw(target, tutorialController.getMode());
}

bool GameplayState::hasHeldGameplayInput() const {
    return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q) ||
           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P) ||
           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R) ||
           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter) ||
           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num1) ||
           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num2) ||
           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num3) ||
           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Backspace) ||
           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift) ||
           sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) ||
           sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
}

void GameplayState::finishTutorialInputGuardIfReleased() {
    if (!tutorialController.isAwaitingInputRelease()) return;
    if (!tutorialController.hasModalKeyHeld() && !hasHeldGameplayInput()) {
        tutorialController.finishInputRelease();
    }
}

const Boss* GameplayState::findLivingBoss() const {
    for (const auto& monster : monsters) {
        if (const auto* boss = dynamic_cast<const Boss*>(monster.get());
            boss && !boss->isDead()) {
            return boss;
        }
    }
    return nullptr;
}

void GameplayState::drawAimingReticle(sf::RenderWindow& target) const {
    if (aimWorldPosition.x < 0.f || aimWorldPosition.y < 0.f ||
        aimWorldPosition.x > GameConfig::HUD_LEFT ||
        aimWorldPosition.y > GameConfig::WINDOW_HEIGHT) {
        return;
    }
    const sf::Vector2f offset = aimWorldPosition - player->getPosition();
    const float distance = std::sqrt(offset.x * offset.x + offset.y * offset.y);
    if (distance > 0.001f) {
        sf::RectangleShape guide({std::min(distance, 360.f), 1.5f});
        guide.setOrigin({0.f, 0.75f});
        guide.setPosition(player->getPosition());
        guide.setRotation(sf::radians(std::atan2(offset.y, offset.x)));
        guide.setFillColor(sf::Color(90, 225, 205, 55));
        target.draw(guide);
    }

    const float pulse = player->canAttack()
        ? 1.f + std::sin(lowHealthPulseTime * 4.f) * 0.08f : 1.f;
    sf::CircleShape ring(12.f);
    ring.setOrigin({12.f, 12.f});
    ring.setPosition(aimWorldPosition);
    ring.setScale({pulse, pulse});
    ring.setFillColor(sf::Color::Transparent);
    ring.setOutlineColor(sf::Color(85, 235, 205, 210));
    ring.setOutlineThickness(1.5f);
    target.draw(ring);

    sf::RectangleShape horizontal({22.f, 1.5f});
    horizontal.setOrigin({11.f, 0.75f});
    horizontal.setPosition(aimWorldPosition);
    horizontal.setFillColor(sf::Color(255, 220, 120, 210));
    target.draw(horizontal);
    sf::RectangleShape vertical({1.5f, 22.f});
    vertical.setOrigin({0.75f, 11.f});
    vertical.setPosition(aimWorldPosition);
    vertical.setFillColor(sf::Color(255, 220, 120, 210));
    target.draw(vertical);
}

void GameplayState::drawLowHealthFeedback(sf::RenderWindow& target) const {
    if (player->getMaxHealth() <= 0.f ||
        player->getHealth() > player->getMaxHealth() * 0.25f) {
        return;
    }
    const auto alpha = static_cast<std::uint8_t>(
        28.f + (std::sin(lowHealthPulseTime * 2.3f) + 1.f) * 12.f);
    const sf::Color red(190, 20, 45, alpha);
    constexpr float mapWidth = GameConfig::HUD_LEFT;
    constexpr float screenHeight = GameConfig::WINDOW_HEIGHT;
    constexpr float edge = 34.f;
    for (const sf::FloatRect area : {
            sf::FloatRect({0.f, 0.f}, {mapWidth, edge}),
            sf::FloatRect({0.f, screenHeight - edge}, {mapWidth, edge}),
            sf::FloatRect({0.f, 0.f}, {edge, screenHeight}),
            sf::FloatRect({mapWidth - edge, 0.f}, {edge, screenHeight})}) {
        sf::RectangleShape vignette(area.size);
        vignette.setPosition(area.position);
        vignette.setFillColor(red);
        target.draw(vignette);
    }
}

void GameplayState::drawPauseOverlay(sf::RenderWindow& target) const {
    const sf::Vector2f viewSize = target.getView().getSize();
    const sf::Vector2f center = target.getView().getCenter();
    sf::RectangleShape dimmer(viewSize);
    dimmer.setPosition(center - viewSize / 2.f);
    dimmer.setFillColor(sf::Color(4, 7, 13, 175));
    target.draw(dimmer);

    sf::RectangleShape panel({420.f, 210.f});
    panel.setOrigin(panel.getSize() / 2.f);
    panel.setPosition(center);
    panel.setFillColor(sf::Color(15, 23, 34, 242));
    panel.setOutlineColor(sf::Color(213, 164, 82));
    panel.setOutlineThickness(2.f);
    target.draw(panel);

    if (!presentationFont.getInfo().family.empty()) {
        sf::Text title(presentationFont, "PAUSED", 38);
        title.setStyle(sf::Text::Bold);
        title.setFillColor(sf::Color(238, 204, 126));
        auto bounds = title.getLocalBounds();
        title.setOrigin({bounds.position.x + bounds.size.x / 2.f,
                         bounds.position.y + bounds.size.y / 2.f});
        title.setPosition(center - sf::Vector2f(0.f, 48.f));
        target.draw(title);

        sf::Text hint(presentationFont,
            "P  RESUME\nWASD  MOVE     LEFT MOUSE  SHOOT\nQ  RADIANT PULSE", 16);
        hint.setFillColor(sf::Color(180, 198, 210));
        hint.setLineSpacing(1.4f);
        bounds = hint.getLocalBounds();
        hint.setOrigin({bounds.position.x + bounds.size.x / 2.f,
                        bounds.position.y + bounds.size.y / 2.f});
        hint.setPosition(center + sf::Vector2f(0.f, 42.f));
        target.draw(hint);
    }
}
