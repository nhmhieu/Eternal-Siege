#include "GameplayState.h"
#include "CombatManager.h"
#include "Sword.h"

#include <iostream>
#include <cmath>

// GameplayState :: GameplayState() {}

GameplayState::GameplayState(sf :: RenderWindow& window) 
    : window(window)
{
    context.allEntity.push_back(&(this->player)); 
    context.players.push_back(&(this->player)) ;  

    Sword* s = new Sword(100, 150) ; 
    player.setCurrentWeapon(s) ; 

    //tao quai vat tam thoi
    monsters.push_back(new Monster(100.0f, 100.0f, 100.0f, 100.0f));
    monsters.push_back(new Monster(200.0f, 300.0f, 100.0f, 100.0f));
    
   
    for (auto* monster : monsters) {
        monster->updateTarget(context.allEntity);
    }
}

GameplayState::~GameplayState() {
    //xoa quai va player 

    for (auto* monster : monsters) {
        delete monster;
    }
    monsters.clear();
}

void GameplayState::onEnter() {
    std::cout << "Giai doan: Buoc vao man choi Gameplay!\n";
}

void GameplayState::onExit() {
    std::cout << "Giai doan: Thoat khoi man choi Gameplay!\n";
}

void GameplayState::handleEvent(const sf::Event& event) {

    //xu li phim bam 
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::P) {
            std::cout << "Tam dung Game (Pause)!\n";
           
        }
    }

    //neu chuot duoc bam
    //Thuc hien tinh toan vi tri cua chuot roi nap vao attDir cua Player de player tan cong
    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {

        if (mousePressed->button == sf::Mouse::Button::Left) {
            sf::Vector2i mousePixel = mousePressed->position;
            sf::Vector2f mouseWorld = this->window.mapPixelToCoords(mousePixel); 

            sf::Vector2f playerPos(player.getX(), player.getY()); 
            

            sf::Vector2f attackDir = mouseWorld - playerPos; 

        
            float length = std::sqrt(attackDir.x * attackDir.x + attackDir.y * attackDir.y);
            if (length != 0.f) {
                attackDir.x /= length; // Ép x về khoảng -1 đến 1
                attackDir.y /= length; // Ép y về khoảng -1 đến 1
            } else {
                attackDir = sf::Vector2f(1.f, 0.f); //Huong mac dinh
            }

           
            player.setAttackDirection(attackDir); 
        }
    }
}

void GameplayState::update(float dt) {
    context.deltaTime = dt ; 

    // 1.----------- Xem Player thực tế có đang di chuyển không
    // std::cout << "Player Real Pos: " << player.getX() << ", " << player.getY() << std::endl;

    // 2.------------ Xem Player lưu trong Context có trùng tọa độ không
    // if (!context.allEntity.empty()) {
    //     std::cout << "Context Player Pos: " << context.allEntity[0]->getX() << ", " << context.allEntity[0]->getY() << std::endl;
    // }

    //---------- 3. Xem deltaTime có lớn hơn 0 không
    // std::cout << "DeltaTime: " << dt << std::endl;


    //Ham nay kiem tra isKeyPressed de player di chuyen
    player.handleInput() ; 


    //Neu Player dang tan cong thi xu li don tan cong
    if(player.getIsAttacking()){
        combatManager.processAttack(&player, player.getCurrentWeapon(), monsters) ; 
    }

    //Ham nay cap nhat vi tri moi cua player theo thoi gian thuc va ve hinh len man hinh
    player.update(context);
    

    for (auto* monster : monsters) {
        monster->update(context) ;

        if(monster->getHealth() == 0){
            std :: cout << "Quai da chet " << endl ; 
            
        }
    }
}

// Hàm update cải tiến nhận thêm tham chiếu window để phục vụ việc lấy tọa độ chuột
void GameplayState::render(sf::RenderWindow& window) {
    // Cập nhật Player ngay trong vòng lặp chính (Dùng delta time đếm ngược từ Clock của Game)
    // Lưu ý: Thông thường hàm update nhân vật sẽ nằm ở hàm update(dt), bạn có thể lưu con trỏ window 
    // hoặc truyền thẳng window vào hàm render này để cập nhật tọa độ chuột trước khi vẽ.
    
    // Ví dụ cập nhật nhanh tọa độ trực tiếp:
    for(auto monster : monsters){
        monster->draw(window) ; 
    }

    //ve hop debug
    if (player.getIsAttacking() && player.getCurrentWeapon() != nullptr) {
        player.getCurrentWeapon()->drawDebug(
            window, 
            { player.getX(), player.getY() }, 
            player.getAttackDirection()
        );
    }

    // Vẽ nhân vật
    this->player.draw(window);
}