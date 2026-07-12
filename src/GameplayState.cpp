#include "GameplayState.h"

#include <iostream>

GameplayState::GameplayState() {
    // 1. Đẩy player vào trước để context sẵn sàng có dữ liệu
    context.allEntity.push_back(&(this->player)); 

    // 2. Tạo quái vật sau
    monsters.push_back(new Monster(100.0f, 100.0f, 100.0f, 100.0f));
    monsters.push_back(new Monster(200.0f, 300.0f, 100.0f, 100.0f));
    
    // 3. Ép tất cả quái vật tìm mục tiêu ngay lập tức khi vừa vào game
    for (auto* monster : monsters) {
        monster->updateTarget(context.allEntity);
    }
}

GameplayState::~GameplayState() {
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
    // Xử lý các sự kiện dạng một lần (như nhấn nút Pause, mở Inventory) theo chuẩn SFML 3
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::P) {
            std::cout << "Tam dung Game (Pause)!\n";
            // Bạn có thể push State Pause vào StateMachine ở đây nếu cần
        }
    }
}

void GameplayState::update(float dt) {
    // Không cần truyền window nếu dùng WASD, nhưng cần nếu dùng cơ chế di chuyển theo chuột
    // Ở đây tạm thời để trống hoặc bạn có thể chỉnh sửa lại tham số tùy ý.
    // Để lấy được tương tác chuột chính xác, ta cập nhật player dựa trên trạng thái phím/chuột.

    // //cmt lại tạm thời
    // context.deltaTime = dt ; 

    // //update player
    // player.update(context);


    // //update quai
    // for (auto* monster : monsters) {
    //     monster->update(context) ;
    // }


    context.deltaTime = dt ; 

    // 1. Xem Player thực tế có đang di chuyển không
    std::cout << "Player Real Pos: " << player.getX() << ", " << player.getY() << std::endl;

    // 2. Xem Player lưu trong Context có trùng tọa độ không
    if (!context.allEntity.empty()) {
        std::cout << "Context Player Pos: " << context.allEntity[0]->getX() << ", " << context.allEntity[0]->getY() << std::endl;
    }

    // 3. Xem deltaTime có lớn hơn 0 không
    std::cout << "DeltaTime: " << dt << std::endl;

    player.update(context);

    for (auto* monster : monsters) {
        monster->update(context) ;
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
    // Vẽ nhân vật
    this->player.draw(window);
}