#include "GameplayState.h"
#include "Sword.h"
#include "Ally.h"
#include "WaveManager.h"
#include <cmath>
#include "Map.h"
#include <iostream>
#include "TextureManager.h"

GameplayState::GameplayState(sf::RenderWindow& window, TextureManager& textureManager, const std::vector<sf::Vector2i>& allyPositions)
    : window(window), textureManager(textureManager) {
    // Thiết lập context
    player = std::make_unique<Player>(textureManager);
    context.allEntity.push_back(player.get());
    context.players.push_back(player.get());

    // Gán vũ khí cho Player
    Sword* sword = new Sword(20, 100);
    player->setCurrentWeapon(sword);

    // Tạo ally từ danh sách vị trí
    std::cout << "Number of ally positions: " << allyPositions.size() << std::endl;

    std::vector<std::string> allyTextureNames = { "Damian", "Evangeline", "Junior", "Lucas" };
    int allyindex = 0;

    for (const auto& pos : allyPositions) {
        std::cout << "Ally at (" << pos.x << ", " << pos.y << ")" << std::endl;
        float x = pos.x * TILE_SIZE + TILE_SIZE / 2.f;
        float y = pos.y * TILE_SIZE + TILE_SIZE / 2.f;

        std::string textureName = allyTextureNames[allyindex % allyTextureNames.size()];
        
        
        auto ally = std::make_unique<Ally>(x, y, textureManager, textureName);
        context.allEntity.push_back(ally.get());
        context.players.push_back(ally.get());
        allies.push_back(std::move(ally));
        allyindex++;
        std::cout << "Ally created at (" << x << ", " << y << ") with texture: " << textureName << std::endl;
    }

    // Tạo quái vật
    //Monster* m1 = new Monster(100, 100, 100, 100);
    //Monster* m2 = new Monster(300, 500, 100, 100);
    //monsters.push_back(m1);
    //monsters.push_back(m2);

    for (auto* m : monsters) {
        context.allEntity.push_back(m);
        context.enemies.push_back(m);
        m->updateTarget(context.players);
    }

    std::cout << "GameplayState khoi tao thanh cong!" << std::endl;
}

GameplayState::~GameplayState() {
    for (auto* m : monsters) delete m;
    monsters.clear();
}

void GameplayState::onEnter() {
    std::cout << "GameplayState: Da vao man choi!" << std::endl;
}

void GameplayState::onExit() {
    std::cout << "GameplayState: Da thoat!" << std::endl;
}

void GameplayState::handleEvent(const sf::Event& event) {
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::P) {
            std::cout << "Tam dung game!" << std::endl;
        }
    }

    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mousePressed->button == sf::Mouse::Button::Left) {
            sf::Vector2i mousePixel = mousePressed->position;
            sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePixel);

            sf::Vector2f playerPos(player->getX(), player->getY());
            sf::Vector2f attackDir = mouseWorld - playerPos;

            float length = std::sqrt(attackDir.x * attackDir.x + attackDir.y * attackDir.y);
            if (length != 0.f) attackDir /= length;
            else attackDir = sf::Vector2f(1.f, 0.f);

            player->setAttackDirection(attackDir);
            player->setIsAttacking(true);   // Sử dụng setter
        }
    }
}

void GameplayState::update(float dt) {
    context.deltaTime = dt;

    // Cập nhật Player
    player->handleInput();
    player->update(context);

    // Xử lý tấn công của Player
    if (player->getIsAttacking()) {
        std::vector<Entity*> targets;
        for (auto* m : monsters) targets.push_back(m);
        combatManager.processAttack(player.get(), player->getCurrentWeapon(), targets);
    }

    context.players.clear();
    context.players.push_back(player.get());
    for (auto& allyPtr : allies) {
        allyPtr->update(context);
    }
    // Cập nhật wavemanager
    waveManager.update(context, monsters);

    // Cập nhật context .enemies(cho ally tìm được quái)
    context.enemies.clear();
    for (auto* m : monsters) {
        context.enemies.push_back(m);
    }
    

    // Cập nhật quái
    for (auto* m : monsters) {
        m->update(context);
    }

    // Xóa quái chết
    auto it = monsters.begin();
    while (it != monsters.end()) {
        if ((*it)->isDead()) {
            delete* it;
            it = monsters.erase(it);
            std::cout << "Quai da bi tieu diet!" << std::endl;
        }
        else {
            ++it;
        }
    }

    // Cập nhật trạng thái tấn công của Player (tự tắt)
    player->updateStatus();
}

void GameplayState::render(sf::RenderWindow& window) {
    player->draw(window);

    for (auto& allyPtr : allies) {
        allyPtr->draw(window);
    }

    for (auto* m : monsters) {
        m->draw(window);
    }
}