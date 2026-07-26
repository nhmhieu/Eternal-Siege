#include "GameplayState.h"
#include "Sword.h"
#include "Ally.h"
#include "WaveManager.h"
#include <cmath>
#include "Map.h"
#include <iostream>
#include "Projectiles.h" 
#include "Arrow.h"
#include "Bow.h"

template <typename T>
void cleanupEntities(std::vector<T*>& entityList) {
    auto it = entityList.begin();
    while (it != entityList.end()) {
        if ((*it)->isDead() && !(*it)->getIsDying()) {
            (*it)->startDying();
            ++it;
        }
        else if ((*it)->isReadyToBeDelete()) {
            delete *it;
            it = entityList.erase(it);
        }
        else {
            ++it;
        }
    }
}

GameplayState::GameplayState(sf::RenderWindow& window, const std::vector<sf::Vector2i>& allyPositions)
    : window(window) {
    
    //day player vao context
    context.allEntity.push_back(&player);
    context.players.push_back(&player);

    // Gán vũ khí cho Player
    // Sword* sword = new Sword(20, 100);
    Bow* bow = new Bow(10, 2.f) ;
    player.setCurrentWeapon(bow);

    // Tạo ally từ danh sách vị trí
    std::cout << "Number of ally positions: " << allyPositions.size() << std::endl;

    for (const auto& pos : allyPositions) {
        std::cout << "Ally at (" << pos.x << ", " << pos.y << ")" << std::endl;
        float x = pos.x * TILE_SIZE + TILE_SIZE / 2.f;
        float y = pos.y * TILE_SIZE + TILE_SIZE / 2.f;

        //khoi tao ally tai vi tri x, y 
        Ally* newAlly = new Ally(x, y) ;

        //gan vu khi la kiem cho ally 
        Sword* s = new Sword(20, 100) ; 
        newAlly->setCurrentWeapon(s) ; 

        //day vao context
        context.allEntity.push_back(newAlly);
        context.players.push_back(newAlly);
        context.allies.push_back(newAlly) ; 
        std::cout << "Ally created at (" << x << ", " << y << ")" << std::endl;
    }

    // Tạo quái vật
    //Monster* m1 = new Monster(100, 100, 100, 100);
    //Monster* m2 = new Monster(300, 500, 100, 100);
    //monsters.push_back(m1);
    //monsters.push_back(m2);

    for (auto* m : context.monsters) {
        Sword* temp = new Sword(20, 100) ; 
        m->setCurrentWeapon(temp) ; 

        context.allEntity.push_back(m);
        context.enemies.push_back(m);
        m->updateTarget(context.players);
    }

    std::cout << "GameplayState khoi tao thanh cong!" << std::endl;
}

GameplayState::~GameplayState() {
    for (auto* m : context.monsters) delete m;
    context.monsters.clear();

    for(auto* p : projectiles) delete p ;
    projectiles.clear() ;
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
        if (mousePressed->button == sf::Mouse::Button::Left){
            if(!player.canAttack()) return ;
            sf::Vector2i mousePixel = mousePressed->position;
            sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePixel);

            sf::Vector2f playerPos(player.getX(), player.getY());
            sf::Vector2f attackDir = mouseWorld - playerPos;

            float length = std::sqrt(attackDir.x * attackDir.x + attackDir.y * attackDir.y);
            if (length != 0.f) attackDir /= length;
            else attackDir = sf::Vector2f(0.f, 0.f);

            player.setAttackDirection(attackDir);
            std :: cout << "Da xac nhan click nay du tieu chuan de tan cong" << std :: endl ;
            player.setIsAttacking(true);   // Sử dụng setter
        }
    }
}

void GameplayState::update(float dt) {
    context.deltaTime = dt;
    context.combatManager = &combatManager ; 

    //lam sach danh sach 2 phe va entity
    context.allEntity.clear();
    context.players.clear();
    context.enemies.clear();

    // 1. Thêm Player vào các danh sách quản lý
    context.allEntity.push_back(&player);
    context.players.push_back(&player);

    for(auto* ally : context.allies){
        context.players.push_back(ally) ; 
        context.allEntity.push_back(ally) ; 
    }

    for(auto* m : context.monsters){
        context.allEntity.push_back(m) ; 
        context.enemies.push_back(m) ; 
        
    }
    
    // Cập nhật wavemanager
    waveManager.update(context, context.monsters);



    for(auto* entity : context.allEntity){
        entity->update(context) ; 
    }


    //Xu li projectile
    if(!context.projectiles.empty()){
        combatManager.processProjectiles(context, context.allEntity) ; 
    }
    else{
        // std :: cout << "Projectiles is empty !!!" << std :: endl ;
    }


    //Don dep va xoa quai da chet
    cleanupEntities(context.monsters) ;
    cleanupEntities(context.allies) ;  

}

void GameplayState::render(sf::RenderWindow& window) {
    player.draw(window);

    if(player.getIsAttacking() && player.canAttack()){
        player.getCurrentWeapon()->drawDebug(window, player.getPosition(), player.getAttackDirection()) ; 
    }

    int count = 0 ;
    for (auto* ally : context.allies) {
        ally->draw(window);

        if(ally->getIsAttacking()){
            ally->getCurrentWeapon()->drawDebug(window, ally->getPosition(), ally->getAttackDirection()) ; 
        }
        else{
            // std :: cout << "Ally "<< count  << " is not attacking" << std :: endl ;
            count++ ; 
        }
    }

    if(!context.projectiles.empty()){
        for(auto* p : context.projectiles){
            window.draw(p->getShape()) ;            
        }
    }

    for (auto* m : context.monsters) {
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