#include "GameplayState.h"
#include "Sword.h"
#include "Ally.h"
#include "WaveManager.h"
#include <cmath>
#include "Map.h"
#include <iostream>
#include "TextureManager.h"
#include "GameOverState.h"
#include "WinState.h"
#include "StateMachine.h"
#include "Projectiles.h"
#include "Arrow.h"
#include "Bow.h"
#include "Constants.h"
#include <algorithm>

using namespace GameConfig;
// ===============================
// UTILITY: XÓA ENTITY CHẾT
// ===============================
template <typename T>
void cleanupEntities(std::vector<T*>& entityList) {
    auto it = entityList.begin();
    while (it != entityList.end()) {
        if ((*it)->isDead() && !(*it)->getIsDying()) {
            (*it)->startDying();
            ++it;
        }
        else if ((*it)->isReadyToBeDelete()) {
            delete* it;
            it = entityList.erase(it);
        }
        else {
            ++it;
        }
    }
}

// ===============================
// CONSTRUCTOR
// ===============================
GameplayState::GameplayState(StateMachine& machine, sf::RenderWindow& window, TextureManager& textureManager, const Map& setupMap, const std::vector<sf::Vector2i>& allyPositions)
    : stateMachine(machine), window(window), textureManager(textureManager), map(setupMap) {

    
    
    // Load texture cho Player
    textureManager.loadTexture("Ash", "assets/images/Ash.png");

    // Tạo Player
    player = std::make_unique<Player>(textureManager);
    context.allEntity.push_back(player.get());
    context.players.push_back(player.get());

    // Gán vũ khí cho Player (Bow)
    player->setCurrentWeapon(std::make_unique<Bow>(10, 2.f));
    player->setAttackCooldown(2.f);

    // Load texture cho Ally
    std::vector<std::string> allyTextureNames = { "Damian", "Evangeline", "Junior", "Lucas" };
    for (const auto& name : allyTextureNames) {
        textureManager.loadTexture(name, "assets/images/" + name + ".png");
    }

    // Tạo Ally từ danh sách vị trí (theo feature/gameplay)
    std::size_t allyIndex = 0;
    for (const auto& pos : allyPositions) {
        if (allyIndex >= allyTextureNames.size()) {
            break;
        }
        float x = pos.x * TILE_SIZE + TILE_SIZE / 2.f;
        float y = pos.y * TILE_SIZE + TILE_SIZE / 2.f;

        // Tạo Ally mới
        Ally* newAlly = new Ally(x, y, textureManager, allyTextureNames[allyIndex]);
        // Gán vũ khí cho Ally
        newAlly->setCurrentWeapon(std::make_unique<Sword>(20, 200));

        // Thêm vào context
        context.allEntity.push_back(newAlly);
        context.players.push_back(newAlly);
        context.allies.push_back(newAlly);

        std::cout << "Ally created at (" << x << ", " << y << ")" << std::endl;
        allyIndex++;
    }

    // Tạo quái vật (nếu có)
    for (auto* m : context.monsters) {
        m->setCurrentWeapon(std::make_unique<Sword>(10, 50));
        context.allEntity.push_back(m);
        context.enemies.push_back(m);
        m->updateTarget(context.players);
    }

    std::cout << "GameplayState init complete! Allies: " << context.allies.size() << std::endl;
}

// ===============================
// DESTRUCTOR
// ===============================
GameplayState::~GameplayState() {
    for (auto* m : context.monsters) delete m;
    context.monsters.clear();

    for (auto* a : context.allies) delete a;
    context.allies.clear();

    for (auto* p : context.projectiles) delete p;
    context.projectiles.clear();
}

// ===============================
// STATE LIFECYCLE
// ===============================
void GameplayState::onEnter()
{
    std::cout << "=== Gameplay Start ===" << std::endl;

    // 1. Spawn Player giữa map
    const float centerX = (DEFAULT_MAP_WIDTH * TILE_SIZE) / 2.f;
    const float centerY = (DEFAULT_MAP_HEIGHT * TILE_SIZE) / 2.f;

    // Đã thêm cặp dấu ngoặc nhọn { }:
    player->setPosition({ centerX, centerY });

}


void GameplayState::onExit() {
    std::cout << "GameplayState: Exited!" << std::endl;
}

// ===============================
// EVENT HANDLING
// ===============================
void GameplayState::handleEvent(const sf::Event& event) {
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::P) {
            paused = !paused;
            std::cout << (paused ? "Game paused!" : "Game resumed!") << std::endl;
            return;
        }
    }

    if (paused) {
        return;
    }

    // Xử lý tấn công của Player (chỉ khi sẵn sàng)
    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mousePressed->button == sf::Mouse::Button::Left) {
            if (!player->canAttack()) return;  // Tối ưu: không bắn khi đang cooldown

            sf::Vector2i mousePixel = mousePressed->position;
            sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePixel);
            sf::Vector2f playerPos(player->getX(), player->getY());
            sf::Vector2f attackDir = mouseWorld - playerPos;

            float length = std::sqrt(attackDir.x * attackDir.x + attackDir.y * attackDir.y);
            if (length <= 0.0001f) {
                return;
            }
            attackDir /= length;

            player->setAttackDirection(attackDir);
            player->setIsAttacking(true);
        }
    }
}

// ===============================
// UPDATE
// ===============================
void GameplayState::update(float dt) {
    if (paused) {
        return;
    }

    context.deltaTime = dt;
    context.combatManager = &combatManager;

    // --- XÂY DỰNG LẠI CONTEXT (mỗi frame) ---
    context.allEntity.clear();
    context.players.clear();
    context.enemies.clear();

    // 1. Thêm Player
    context.allEntity.push_back(player.get());
    context.players.push_back(player.get());

    // 2. Thêm Ally
    for (auto* ally : context.allies) {
        context.players.push_back(ally);
        context.allEntity.push_back(ally);
    }

    // 3. Thêm Monster
    for (auto* m : context.monsters) {
        context.enemies.push_back(m);
        context.allEntity.push_back(m);
    }

    // 4. Cập nhật WaveManager (sinh quái)
    waveManager.update(context, context.monsters);

    // Add monsters spawned this frame before entity updates.
    for (auto* monster : context.monsters) {
        if (std::find(context.enemies.begin(), context.enemies.end(), monster) == context.enemies.end()) {
            context.enemies.push_back(monster);
            context.allEntity.push_back(monster);
        }
    }

    // 5. Cập nhật tất cả Entity
    for (auto* entity : context.allEntity) {
        entity->update(context);
    }

    // 6. Xử lý đạn (Projectiles)
    if (!context.projectiles.empty()) {
        combatManager.processProjectiles(context, context.allEntity);
    }


    // 7. Xóa Entity chết (dùng template)
    cleanupEntities(context.monsters);
    cleanupEntities(context.allies);

    // 8. Kiểm tra GameOver (Player chết)
    if (player->isDead()) {
        std::cout << "Player died! Game Over!" << std::endl;
        stateMachine.changeState(std::make_unique<GameOverState>(stateMachine, window, textureManager));
        return;
    }

    // 9. Kiểm tra Win (hoàn thành wave)
    if (waveManager.isGameCompleted() && context.monsters.empty()) {
        std::cout << "All waves completed! Victory!" << std::endl;
        stateMachine.changeState(std::make_unique<WinState>(stateMachine, window, textureManager));
        return;
    }
}

// ===============================
// RENDER
// ===============================
void GameplayState::render(sf::RenderWindow& window) {
    // Vẽ map/background (nếu có)
     map.draw(window);
    // --- Vẽ Player ---
    player->draw(window);
    player->drawHealthBar(window);

    // --- Vẽ Ally ---
    for (auto* ally : context.allies) {
        ally->draw(window);
        ally->drawHealthBar(window);
    }

    // --- Vẽ Monster ---
    for (auto* m : context.monsters) {
        m->draw(window);
        m->drawHealthBar(window);
    }

    // --- Vẽ Projectiles ---
    for (auto* p : context.projectiles) {
        window.draw(p->getShape());
    }

    // --- DEBUG: Vẽ hitbox khi đang tấn công ---
    if (player->getIsAttacking() && player->getCurrentWeapon()) {
        player->getCurrentWeapon()->drawDebug(window, player->getPosition(), player->getAttackDirection());
    }

    for (auto* ally : context.allies) {
        if (ally->getIsAttacking()) {
            ally->getCurrentWeapon()->drawDebug(window, ally->getPosition(), ally->getAttackDirection());
        }
    }

    for (auto* m : context.monsters) {
        if (m->getIsAttacking() && m->getCurrentWeapon()) {
            m->getCurrentWeapon()->drawDebug(window, m->getPosition(), m->getAttackDirection());
        }
    }
}
