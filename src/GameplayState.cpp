#include "GameplayState.h"
#include "Constants.h"
#include <iostream>

using namespace GameConfig;

GameplayState::GameplayState(
    const Map& setupMap,
    const std::vector<sf::Vector2i>& positions)
    : map(setupMap),
      allyPositions(positions)
{
}

void GameplayState::onEnter()
{
    std::cout << "=== Gameplay Start ===" << std::endl;

    // 1. Spawn Player giữa map
    const float centerX = (DEFAULT_MAP_WIDTH * TILE_SIZE) / 2.f;
    const float centerY = (DEFAULT_MAP_HEIGHT * TILE_SIZE) / 2.f;
    
    // Đã thêm cặp dấu ngoặc nhọn { }:
    player.setPosition({centerX, centerY});

    // 2. TẠO 4 TƯỚNG NPC (ALLY) TỪ DANH SÁCH VỊ TRÍ ĐÃ CHỌN
    allies.clear();
    for (const auto& gridPos : allyPositions)
    {
        float worldX = gridPos.x * TILE_SIZE + (TILE_SIZE / 2.f);
        float worldY = gridPos.y * TILE_SIZE + (TILE_SIZE / 2.f);

        allies.push_back(std::make_unique<Ally>(worldX, worldY));
    }
}

void GameplayState::onExit()
{
    std::cout << "Exit Gameplay" << std::endl;
}

void GameplayState::handleEvent(const sf::Event& event)
{
    if (const auto* key = event.getIf<sf::Event::KeyPressed>())
    {
        if (key->code == sf::Keyboard::Key::Escape)
        {
            std::cout << "ESC pressed" << std::endl;
        }
    }
}


void GameplayState::update(float dt)
{
    // 1. Update Player (Chỉ truyền dt, giống như đã làm ở bài trước)
    player.update(dt);

    // 2. Update tất cả các tướng NPC
    for (auto& ally : allies)
    {
        ally->update(dt);
    }
}

void GameplayState::render(sf::RenderWindow& window)
{
    // 1. Vẽ Map
    map.draw(window);

    // 2. Vẽ tất cả các tướng NPC (và tầm đánh của họ)
    for (const auto& ally : allies)
    {
        ally->render(window);
    }

    // 3. Vẽ Player
    player.render(window);
}