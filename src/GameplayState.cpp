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
GameplayState::GameplayState(StateMachine& machine, sf::RenderWindow& window, TextureManager& textureManager, const std::vector<sf::Vector2i>& allyPositions)
    : stateMachine(machine), window(window), textureManager(textureManager), map(15, 15) {

    // Load texture cho Player
    textureManager.loadTexture("Ash", "assets/images/Ash.png");

    // Tạo Player
    player = std::make_unique<Player>(textureManager);
    context.allEntity.push_back(player.get());
    context.players.push_back(player.get());

    // Gán vũ khí cho Player (Bow)
    player->setCurrentWeapon(std::make_unique<Bow>(10, 2.f));;

    // Load texture cho Ally
    std::vector<std::string> allyTextureNames = { "Damian", "Evangeline", "Junior", "Lucas" };
    for (const auto& name : allyTextureNames) {
        textureManager.loadTexture(name, "assets/images/" + name + ".png");
    }

    // Tạo Ally từ danh sách vị trí (theo feature/gameplay)
    int allyIndex = 0;
    for (const auto& pos : allyPositions) {
        float x = pos.x * TILE_SIZE + TILE_SIZE / 2.f;
        float y = pos.y * TILE_SIZE + TILE_SIZE / 2.f;

        // Tạo Ally mới
        Ally* newAlly = new Ally(x, y, textureManager, allyTextureNames[allyIndex]);
        // Gán vũ khí cho Ally
        newAlly->setCurrentWeapon(std::make_unique<Sword>(20, 100));

        // Thêm vào context
        context.allEntity.push_back(newAlly);
        context.players.push_back(newAlly);
        context.allies.push_back(newAlly);

        std::cout << "Ally created at (" << x << ", " << y << ")" << std::endl;
        allyIndex++;
    }

    // Tạo quái vật (nếu có)
    for (auto* m : context.monsters) {
        m->setCurrentWeapon(std::make_unique<Sword>(20, 100));
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
void GameplayState::onEnter() {
    std::cout << "GameplayState: Entered!" << std::endl;
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
            std::cout << "Pause game!" << std::endl;
        }
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
            if (length != 0.f) attackDir /= length;
            else attackDir = sf::Vector2f(0.f, 0.f);

            player->setAttackDirection(attackDir);
            player->setIsAttacking(true);
        }
    }
}

// ===============================
// UPDATE
// ===============================
void GameplayState::update(float dt) {
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
    if (player->getIsAttacking() && player->canAttack()) {
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