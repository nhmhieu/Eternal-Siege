#include "Map.h"
#include "Constants.h" // Sử dụng duy nhất nguồn này cho cấu hình
#include <iostream>
#include <random>

using namespace GameConfig;

Map::Map(int width, int height)
{
    this->width = width;
    this->height = height;

    generate();
}

void Map::generate()
{
    tiles.assign(height, std::vector<int>(width, TILE_WALL));

    std::random_device rd;
    std::mt19937 gen(rd());

    int x = width / 2;
    int y = height / 2;

    std::uniform_int_distribution<> dir(0, 3);

    for (int i = 0; i < width * height * 4; i++)
    {
        // Tạo tỷ lệ: 20% là đường mòn (Path), 80% là cỏ (Grass)
        if (i % 5 == 0) {
            tiles[y][x] = TILE_PATH;
        } else {
            tiles[y][x] = TILE_GRASS;
        }

        // Đã xóa phần code bị trùng lặp ở đây
        switch (dir(gen))
        {
        case 0:
            x++;
            break;
        case 1:
            x--;
            break;
        case 2:
            y++;
            break;
        case 3:
            y--;
            break;
        }

        if (x < 1)
            x = 1;
        if (x > width - 2)
            x = width - 2;

        if (y < 1)
            y = 1;
        if (y > height - 2)
            y = height - 2;
    }
}

void Map::draw(sf::RenderWindow& window)
{
    // Đồng bộ sử dụng TILE_SIZE từ GameConfig và viết gọn bằng cú pháp { }
    sf::RectangleShape tile({TILE_SIZE, TILE_SIZE});

    tile.setOutlineColor(sf::Color::Black);
    tile.setOutlineThickness(-1.f);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if (tiles[y][x] == TILE_GRASS)
            {
                tile.setFillColor(sf::Color::Green);
            }
            else if (tiles[y][x] == TILE_WALL)
            {
                tile.setFillColor(sf::Color(100, 60, 20));
            }
            else if (tiles[y][x] == TILE_PATH)
            {
                tile.setFillColor(sf::Color::Yellow);
            }
            else if (tiles[y][x] == TILE_SPAWN)
            {
                tile.setFillColor(sf::Color::Blue);
            }

            // Đồng bộ tọa độ bằng cú pháp { } của SFML 3
            tile.setPosition({x * TILE_SIZE, y * TILE_SIZE});

            window.draw(tile);
        }
    }
}

void Map::handleMouseClick(
    int mouseX,
    int mouseY,
    std::vector<sf::Vector2i>& selectedPositions,
    int maxAllies)
{
    // Đồng bộ phép chia lưới dựa theo cấu hình chuẩn
    int gridX = static_cast<int>(mouseX / TILE_SIZE);
    int gridY = static_cast<int>(mouseY / TILE_SIZE);

    if (gridX >= 0 &&
        gridX < width &&
        gridY >= 0 &&
        gridY < height)
    {
        if (static_cast<int>(selectedPositions.size()) < maxAllies &&
            (tiles[gridY][gridX] == TILE_GRASS ||
             tiles[gridY][gridX] == TILE_PATH))
        {
            tiles[gridY][gridX] = TILE_SPAWN;

            selectedPositions.push_back(
                sf::Vector2i(gridX, gridY));

            spawnCount++;

            std::cout
                << "Spawn placed at ("
                << gridX
                << ", "
                << gridY
                << ")\n";

            std::cout
                << "Spawn count: "
                << spawnCount
                << "/"
                << maxAllies
                << "\n";
        }
        else
        {
            std::cout
                << "Cannot place spawn here!\n";
        }
    }
}

const std::vector<sf::Vector2i>& Map::getSelectedPositions() const
{
    return selectedPositions;
}

bool Map::canStart() const
{
    return spawnCount == 4;
}