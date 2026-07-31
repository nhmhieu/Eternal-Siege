#include "GameplayState.h"

#include "BalanceConfig.h"
#include "Bow.h"
#include "Constants.h"
#include "GameOverState.h"
#include "Sword.h"
#include "WinState.h"
#include "GameContext.h"
#include <algorithm>
#include <cmath>
#include <iostream>

using namespace GameConfig;

GameplayState::GameplayState(
    StateMachine& machine,
    sf::RenderWindow& gameWindow,
    TextureManager& textures,
    const Map& setupMap,
    const std::vector<sf::Vector2i>& selectedAllyPositions)
    : stateMachine(machine),
      window(gameWindow),
      textureManager(textures),
      map(setupMap),
      allyPositions(selectedAllyPositions) {

    textureManager.loadTexture("Ash", "assets/images/Ash.png");
    for (const std::string name : {"Damian", "Evangeline", "Junior", "Lucas"}) {
        textureManager.loadTexture(name, "assets/images/" + name + ".png");
    }

    player = std::make_unique<Player>(textureManager);
    player->setCurrentWeapon(std::make_unique<Bow>(
        static_cast<float>(BalanceConfig::PLAYER_DAMAGE), 0.55f));
    player->setAttackCooldown(0.55f);
}

void GameplayState::onEnter() {
    std::cout << "=== Gameplay Start ===\n";
    paused = false;
    hud.load();

    const sf::Vector2i centerCell =
        map.nearestWalkable({map.getWidth() / 2, map.getHeight() / 2});
    player->setPosition(map.gridToWorld(centerCell));

    allies.clear();
    static const std::vector<std::string> names = {
        "Damian", "Evangeline", "Junior", "Lucas"
    };

    for (std::size_t i = 0; i < allyPositions.size() && i < names.size(); ++i) {
        const sf::Vector2f worldPosition = map.gridToWorld(allyPositions[i]);
        auto ally = std::make_unique<Ally>(
            worldPosition.x, worldPosition.y, textureManager, names[i]);
        ally->setCurrentWeapon(std::make_unique<Sword>(
            BalanceConfig::ALLY_DAMAGE, 105.f));
        allies.push_back(std::move(ally));
    }

    context.combatManager = &combatManager;
    context.map = &map;
    rebuildContext();
}

void GameplayState::onExit() {
    std::cout << "GameplayState: Exited!\n";
}

void GameplayState::rebuildContext() {
    context.allEntity.clear();
    context.players.clear();
    context.enemies.clear();
    context.monsters.clear();
    context.allies.clear();

    context.allEntity.push_back(player.get());
    context.players.push_back(player.get());

    for (auto& ally : allies) {
        context.allEntity.push_back(ally.get());
        context.players.push_back(ally.get());
        context.allies.push_back(ally.get());
    }

    for (auto& monster : monsters) {
        context.allEntity.push_back(monster.get());
        context.enemies.push_back(monster.get());
        context.monsters.push_back(monster.get());
    }
}

void GameplayState::collectRewardsAndRemoveDead() {
    for (auto& monster : monsters) {
        if (monster->isDead()) {
            upgradeManager.addGold(monster->claimGoldReward());
        }
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
        }
    }
}

void GameplayState::handleEvent(const sf::Event& event) {
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::P) {
            paused = !paused;
            return;
        }

        if (paused) {
            return;
        }

        if (key->code == sf::Keyboard::Key::Enter) {
            startNextWave();
            return;
        }

        if (waveManager.isIntermission()) {
            if (key->code == sf::Keyboard::Key::Num1) {
                upgradeManager.purchase(UpgradeType::Damage, *player, allies);
            } else if (key->code == sf::Keyboard::Key::Num2) {
                upgradeManager.purchase(UpgradeType::Vitality, *player, allies);
            } else if (key->code == sf::Keyboard::Key::Num3) {
                upgradeManager.purchase(UpgradeType::FireRate, *player, allies);
            } else if (key->code == sf::Keyboard::Key::Backspace) {
                upgradeManager.undoLastPurchase(*player, allies);
            }
        }
    }

    if (paused) {
        return;
    }

    if (const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouse->button != sf::Mouse::Button::Left || !player->canAttack()) {
            return;
        }

        const sf::Vector2f mouseWorld = window.mapPixelToCoords(mouse->position);
        sf::Vector2f attackDirection = mouseWorld - player->getPosition();
        const float length = std::sqrt(
            attackDirection.x * attackDirection.x +
            attackDirection.y * attackDirection.y);
        if (length <= 0.0001f) return;

        attackDirection /= length;
        player->setAttackDirection(attackDirection);
        player->setIsAttacking(true);
    }
}

void GameplayState::update(float dt) {
    if (paused) {
        return;
    }

    // Tranh mot frame qua lon khi keo/cat cua so lam entity "teleport".
    context.deltaTime = std::min(dt, 0.05f);
    context.combatManager = &combatManager;
    context.map = &map;

    rebuildContext();

    const bool noMonstersAlive = std::none_of(
        monsters.begin(),
        monsters.end(),
        [](const std::unique_ptr<Monster>& monster) {
            return monster && !monster->isDead();
        });

    if (auto spawned = waveManager.update(
            context.deltaTime, map, noMonstersAlive)) {
        monsters.push_back(std::move(spawned));
        rebuildContext();
    }

    // allEntity la snapshot observer cua frame hien tai.
    for (Entity* entity : context.allEntity) {
        if (entity) entity->update(context);
    }

    combatManager.processProjectiles(context, context.allEntity);
    collectRewardsAndRemoveDead();
    rebuildContext();

    hud.update(*player, waveManager, upgradeManager,
               static_cast<int>(monsters.size()));

    if (player->isDead()) {
        stateMachine.changeState(std::make_unique<GameOverState>(
            stateMachine, window, textureManager));
        return;
    }

    if (waveManager.isGameCompleted() && monsters.empty()) {
        stateMachine.changeState(std::make_unique<WinState>(
            stateMachine, window, textureManager));
    }
}

void GameplayState::render(sf::RenderWindow& target) {
    map.draw(target);

    player->draw(target);
    player->drawHealthBar(target);

    for (const auto& ally : allies) {
        ally->draw(target);
        ally->drawHealthBar(target);
    }

    for (const auto& monster : monsters) {
        monster->draw(target);
        monster->drawHealthBar(target);
    }

    for (const auto& projectile : context.projectiles) {
        target.draw(projectile->getShape());
    }

    hud.draw(target);
}
