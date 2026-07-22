#include "GameplayState.h"
#include "Sword.h"
#include "Ally.h"
#include "WaveManager.h"
#include <cmath>
#include "Map.h"
#include <iostream>
#include "TextureManager.h"

GameplayState::GameplayState(sf::RenderWindow& window, TextureManager& textureManager, const std::vector<sf::Vector2i>& allyPositions)
    : window(window), textureManager(textureManager), map(15, 15) {
    textureManager.loadTexture("Ash", "assets/images/Ash.png");

    player = std::make_unique<Player>(textureManager);
    context.allEntity.push_back(player.get());
    context.players.push_back(player.get());

    sword = std::make_unique<Sword>(20, 100);
    player->setCurrentWeapon(sword.get());

    std::vector<std::string> allyTextureNames = { "Damian", "Evangeline", "Junior", "Lucas" };
    for (const auto& name : allyTextureNames) {
        textureManager.loadTexture(name, "assets/images/" + name + ".png");
    }
    int allyindex = 0;

    for (const auto& pos : allyPositions) {
        float x = pos.x * TILE_SIZE + TILE_SIZE / 2.f;
        float y = pos.y * TILE_SIZE + TILE_SIZE / 2.f;

        std::string textureName = allyTextureNames[allyindex % allyTextureNames.size()];
        
        auto ally = std::make_unique<Ally>(x, y, textureManager, textureName);
        context.allEntity.push_back(ally.get());
        context.players.push_back(ally.get());
        allies.push_back(std::move(ally));
        allyindex++;
    }

    std::cout << "Total allies: " << allies.size() << std::endl;

    for (auto* m : monsters) {
        context.allEntity.push_back(m);
        context.enemies.push_back(m);
        m->updateTarget(context.players);
    }

    std::cout << "GameplayState khoi tao thanh cong!" << std::endl;
    std::cout << "Ally count at init=" << allies.size() << std::endl;
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

    player->updateStatus();

    context.players.clear();
    context.players.push_back(player.get());
    for (auto& allyPtr : allies) {
        context.players.push_back(allyPtr.get());
    }

    // Cập nhật wavemanager trước khi ally update
    waveManager.update(context, monsters);

    context.enemies.clear();
    context.allEntity.clear();
    context.allEntity.push_back(player.get());
    for (auto& allyPtr : allies) {
        context.allEntity.push_back(allyPtr.get());
    }
    for (auto* m : monsters) {
        context.enemies.push_back(m);
        context.allEntity.push_back(m);
    }

    for (auto& allyPtr : allies) {
        allyPtr->update(context);
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

}

void GameplayState::render(sf::RenderWindow& window) {
    // Vẽ map/background trước (lấp đầy màn hình)
    map.draw(window);

    // Vẽ player
    player->draw(window);

    // Vẽ ally
    for (auto& allyPtr : allies) {
        allyPtr->draw(window);
    }

    // Vẽ monster
    for (auto* m : monsters) {
        m->draw(window);
    }
}
