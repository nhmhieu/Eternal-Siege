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
        Sword* temp = new Sword(20, 100) ; 
        m->setCurrentWeapon(temp) ; 

        context.allEntity.push_back(m);
        context.enemies.push_back(m);
        m->updateTarget(context.players);
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

    //can toi uu lai cho nay chi tinh attackDir khi player that su co the tan cong (cooldownTimer <= 0)
    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mousePressed->button == sf::Mouse::Button::Left) {
            sf::Vector2i mousePixel = mousePressed->position;
            sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePixel);

            sf::Vector2f playerPos(player.getX(), player.getY());
            sf::Vector2f attackDir = mouseWorld - playerPos;

            float length = std::sqrt(attackDir.x * attackDir.x + attackDir.y * attackDir.y);
            if (length != 0.f) attackDir /= length;
            else attackDir = sf::Vector2f(0.f, 0.f);

            player.setAttackDirection(attackDir);
            // player.setIsAttacking(true);   // Sử dụng setter
        }
    }
}

void GameplayState::update(float dt) {
    context.deltaTime = dt;

    // Cập nhật Player
    player.handleInput();
    player.update(context);

    // Xử lý tấn công của Player
    // if(player.canAttack()){
    //     player.startAttacking() ;
    // }
    player.canAttack() ;
    if (player.getIsAttacking() && player.canAttack()) {
        std :: cout << "Player is Aattacking" << std :: endl ;
        combatManager.processAttack(&player, player.getCurrentWeapon(), context.enemies);

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

        if(m->canAttack()){
            // std :: cout << "Quai bat dau tan cong !" << std :: endl ;
            m->startAttacking() ; 
        }
        if(m->getIsAttacking()){
            combatManager.processAttack(m, m->getCurrentWeapon(), context.players) ; 

        }
        
    }

    int count = 0 ; 
    
    for(auto& ally : allies){
        ally.update(context) ;

        if(ally.canAttack()){
            ally.startAttacking() ; 
        }

        if(ally.getIsAttacking()){

            combatManager.processAttack(&ally, ally.getCurrentWeapon(), context.enemies) ; 
            std :: cout << "Ally is attacking, target health :  " << ally.getHealth() << std :: endl ;
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
    auto p_monster = monsters.begin();
    while (p_monster != monsters.end()) {
        if ((*p_monster)->isDead() && !(*p_monster)->getIsDying()) {
            (*p_monster)->startDying() ; //bat isDying = true va xu li animation chet
            p_monster++ ; 
            std :: cout << "Quai bat dau chet" << std :: endl ; 
        }
        else if((*p_monster)->isReadyToBeDelete()){
            p_monster = monsters.erase(p_monster) ;
            std :: cout << "Quai da chet va xoa quai khoi mang" << std :: endl ; 
        }
        else {
            ++p_monster;
        }
    }

    auto p_ally = allies.begin() ;
    while(p_ally != allies.end()){

        //neu ally nay da het mau ma chua bat isDying thi bat flag isDying va chay animation
        if(p_ally->isDead() && !(p_ally->getIsDying())){
            p_ally->startDying() ; 
            p_ally++ ; 
            std :: cout << "An ally has been slain, dying animation start !" << std :: endl ; 

        }
        else if(p_ally->isReadyToBeDelete()){
            p_ally = allies.erase(p_ally) ; 
            std :: cout << "Ally has died and removed from vector" << std :: endl ; 
        }
        else{ //ally chua chet thi duyet ally tiep theo
            ++p_ally ; 
        }

    }

    // Cập nhật trạng thái tấn công của Player (tự tắt)
    player.updateStatus();
}

void GameplayState::render(sf::RenderWindow& window) {
    player.draw(window);

    if(player.getIsAttacking() && player.canAttack()){
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

        // std :: cout << "Dang chuan bi ve debug" << std :: endl ; 
        if(!m->getCurrentWeapon()){
            // std :: cout << "Quai khong co vu khi !!!" << std :: endl ; 
        }

        if(m->getIsAttacking()){
            m->getCurrentWeapon()->drawDebug(window, m->getPosition(), m->getAttackDirection()) ; 
        }
    }
}