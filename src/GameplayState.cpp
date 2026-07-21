#include "GameplayState.h"
#include "Sword.h"
#include "Ally.h"
#include "WaveManager.h"
#include <cmath>
#include "Map.h"
#include <iostream>

GameplayState::GameplayState(sf::RenderWindow& window, const std::vector<sf::Vector2i>& allyPositions)
    : window(window) {
    // Thiết lập context
    context.allEntity.push_back(&player);
    context.players.push_back(&player);

    // Gán vũ khí cho Player
    Sword* sword = new Sword(20, 100);
    player.setCurrentWeapon(sword);

    // Tạo ally từ danh sách vị trí
    std::cout << "Number of ally positions: " << allyPositions.size() << std::endl;

    for (const auto& pos : allyPositions) {
        std::cout << "Ally at (" << pos.x << ", " << pos.y << ")" << std::endl;
        float x = pos.x * TILE_SIZE + TILE_SIZE / 2.f;
        float y = pos.y * TILE_SIZE + TILE_SIZE / 2.f;
        allies.emplace_back(x, y);
        context.allEntity.push_back(&allies.back());
        context.players.push_back(&allies.back());
        std::cout << "Ally created at (" << x << ", " << y << ")" << std::endl;
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
        m->setCurrentWeapon(sword) ; 
    }

    for(auto& ally : allies){
        Sword* temp = new Sword(20, 100) ;  

        ally.setCurrentWeapon(temp) ; 
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

            sf::Vector2f playerPos(player.getX(), player.getY());
            sf::Vector2f attackDir = mouseWorld - playerPos;

            float length = std::sqrt(attackDir.x * attackDir.x + attackDir.y * attackDir.y);
            if (length != 0.f) attackDir /= length;
            else attackDir = sf::Vector2f(1.f, 0.f);

            player.setAttackDirection(attackDir);
            player.setIsAttacking(true);   // Sử dụng setter
        }
    }
}

void GameplayState::update(float dt) {
    context.deltaTime = dt;

    // Cập nhật Player
    player.handleInput();
    player.update(context);

    // Xử lý tấn công của Player
    if (player.getIsAttacking()) {
        std::vector<Entity*> targets;
        for (auto* m : monsters) targets.push_back(m);
        combatManager.processAttack(&player, player.getCurrentWeapon(), targets);

    }

    context.players.clear();
    context.players.push_back(&player);
    for (auto& ally : allies) {
        context.players.push_back(&ally);
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

        // if(m->getIsAttacking()){
        //     combatManager.processAttack(m, m->getCurrentWeapon(), context.players) ; 
        //     std :: cout << "Monster is attacking ! " << std :: endl ; 
        // }
        // else{
        //     std :: cout << "Monster is not attacking" << std :: endl ; 
        // }
        // std :: cout << "Da chay duoc den truoc m->updateStatus" << std :: endl ; 

        // m->updateStatus() ;

    }

    int count = 0 ; 
    
    for(auto& ally : allies){
        ally.update(context) ;

        if(ally.getTarget()){
            // std::cout << "Ally [" << &ally << "] Target: [" << ally.getTarget() << "]" << std::endl;

        }

        // std :: cout << "Kiem tra ally co dang danh khong" << std :: endl ;

        if(ally.getIsAttacking()){

            combatManager.processAttack(&ally, ally.getCurrentWeapon(), context.enemies) ; 
            // std :: cout << "Ally is attacking ! " << count + 1 << std :: endl ; 
            // std :: cout << ally.getAttackClock().getElapsedTime().asSeconds() << std :: endl ; 
        }
        else{
            // std :: cout << "Ally is not attacking : " << count << std :: endl ; 
        }

        count++ ;

        ally.updateStatus() ;

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
    player.updateStatus();
}

void GameplayState::render(sf::RenderWindow& window) {
    player.draw(window);

    if(player.getIsAttacking()){
        player.getCurrentWeapon()->drawDebug(window, player.getPosition(), player.getAttackDirection()) ; 
    }

    for (auto& ally : allies) {
        ally.draw(window);

        if(ally.getIsAttacking()){
            ally.getCurrentWeapon()->drawDebug(window, ally.getPosition(), ally.getAttackDirection()) ; 
        }
    }

    for (auto* m : monsters) {
        m->draw(window);

        // if(m->getIsAttacking()){
        //     m->getCurrentWeapon()->drawDebug(window, m->getPosition(), m->getAttackDirection()) ; 
        // }
    }
}