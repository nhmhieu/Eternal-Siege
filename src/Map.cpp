#include "Map.h"
#include "Constants.h"

#include <algorithm>
#include <array>
#include <queue>

using namespace GameConfig;

namespace {
int flatten(sf::Vector2i cell, int width) {
    return cell.y * width + cell.x;
}
}

Map::Map(int mapWidth, int mapHeight)
    : width(mapWidth), height(mapHeight) {
    generate();
}

void Map::generate() {
    tiles.assign(height, std::vector<TileType>(width, TILE_GRASS));

    for (int x = 0; x < width; ++x) {
        tiles.front()[x] = TILE_WALL;
        tiles.back()[x] = TILE_WALL;
    }
    for (int y = 0; y < height; ++y) {
        tiles[y].front() = TILE_WALL;
        tiles[y].back() = TILE_WALL;
    }

    // Hai dải tường có cổng tạo mê cung đơn giản nhưng luôn có đường đi.
    if (width >= 12 && height >= 12) {
        for (int y = 2; y < height - 2; ++y) {
            if (y != 4 && y != 10) tiles[y][4] = TILE_WALL;
        }
        for (int y = 2; y < height - 2; ++y) {
            if (y != 6 && y != 11) tiles[y][9] = TILE_WALL;
        }
    }

    // Đường chính giúp người chơi nhìn được tuyến phòng thủ.
    const int middleY = height / 2;
    for (int x = 1; x < width - 1; ++x) {
        if (tiles[middleY][x] != TILE_WALL) tiles[middleY][x] = TILE_PATH;
    }

    enemySpawnCells = {
        {1, 1},
        {width - 2, 1},
        {1, height - 2},
        {width - 2, height - 2}
    };
    for (const auto cell : enemySpawnCells) {
        tiles[cell.y][cell.x] = TILE_SPAWN;
    }
}

bool Map::isInside(sf::Vector2i cell) const {
    return cell.x >= 0 && cell.x < width && cell.y >= 0 && cell.y < height;
}

bool Map::isWalkable(int x, int y) const {
    return isWalkable({x, y});
}

bool Map::isWalkable(sf::Vector2i cell) const {
    return isInside(cell) && tiles[cell.y][cell.x] != TILE_WALL;
}

sf::Vector2i Map::worldToGrid(sf::Vector2f world) const {
    return {
        static_cast<int>(world.x / TILE_SIZE),
        static_cast<int>(world.y / TILE_SIZE)
    };
}

sf::Vector2f Map::gridToWorld(sf::Vector2i cell) const {
    return {
        cell.x * TILE_SIZE + TILE_SIZE * 0.5f,
        cell.y * TILE_SIZE + TILE_SIZE * 0.5f
    };
}

bool Map::isWalkableWorld(sf::Vector2f center, float halfSize) const {
    const std::array<sf::Vector2f, 4> corners = {{
        {center.x - halfSize, center.y - halfSize},
        {center.x + halfSize, center.y - halfSize},
        {center.x - halfSize, center.y + halfSize},
        {center.x + halfSize, center.y + halfSize}
    }};
    return std::all_of(corners.begin(), corners.end(),
        [this](sf::Vector2f point) { return isWalkable(worldToGrid(point)); });
}

sf::Vector2i Map::nearestWalkable(sf::Vector2i cell) const {
    if (isWalkable(cell)) return cell;

    std::queue<sf::Vector2i> frontier;
    std::vector<bool> visited(width * height, false);
    if (!isInside(cell)) {
        cell.x = std::clamp(cell.x, 0, width - 1);
        cell.y = std::clamp(cell.y, 0, height - 1);
    }
    frontier.push(cell);
    visited[flatten(cell, width)] = true;

    constexpr std::array<sf::Vector2i, 4> directions = {{
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}
    }};
    while (!frontier.empty()) {
        const auto current = frontier.front();
        frontier.pop();
        for (const auto direction : directions) {
            const auto next = current + direction;
            if (!isInside(next) || visited[flatten(next, width)]) continue;
            if (isWalkable(next)) return next;
            visited[flatten(next, width)] = true;
            frontier.push(next);
        }
    }
    return {1, 1};
}

std::vector<sf::Vector2i> Map::findPathBFS(
    sf::Vector2i start, sf::Vector2i goal) const {
    start = nearestWalkable(start);
    goal = nearestWalkable(goal);

    const int cellCount = width * height;
    std::vector<int> parent(cellCount, -1);
    std::vector<bool> visited(cellCount, false);
    std::queue<sf::Vector2i> frontier;

    frontier.push(start);
    visited[flatten(start, width)] = true;

    constexpr std::array<sf::Vector2i, 4> directions = {{
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}
    }};

    while (!frontier.empty()) {
        const auto current = frontier.front();
        frontier.pop();
        if (current == goal) break;

        for (const auto direction : directions) {
            const auto next = current + direction;
            if (!isWalkable(next)) continue;
            const int nextIndex = flatten(next, width);
            if (visited[nextIndex]) continue;
            visited[nextIndex] = true;
            parent[nextIndex] = flatten(current, width);
            frontier.push(next);
        }
    }

    if (!visited[flatten(goal, width)]) return {};

    std::vector<sf::Vector2i> reversed;
    for (int at = flatten(goal, width); at != -1; at = parent[at]) {
        reversed.push_back({at % width, at / width});
        if (at == flatten(start, width)) break;
    }
    std::reverse(reversed.begin(), reversed.end());
    return reversed;
}

void Map::handleMouseClick(
    int mouseX, int mouseY,
    std::vector<sf::Vector2i>& selectedPositions,
    int maxAllies) {
    const sf::Vector2i cell = worldToGrid(
        {static_cast<float>(mouseX), static_cast<float>(mouseY)});
    if (!isInside(cell) || !isWalkable(cell)) return;
    if (tiles[cell.y][cell.x] == TILE_SPAWN) return;

    const auto existing = std::find(selectedPositions.begin(),
                                    selectedPositions.end(), cell);
    if (existing != selectedPositions.end()) {
        selectedPositions.erase(existing);
        tiles[cell.y][cell.x] = TILE_GRASS;
        return;
    }
    if (static_cast<int>(selectedPositions.size()) >= maxAllies) return;

    selectedPositions.push_back(cell);
    tiles[cell.y][cell.x] = TILE_DEPLOY;
}

void Map::draw(sf::RenderWindow& window) const {
    sf::RectangleShape tile({TILE_SIZE, TILE_SIZE});
    tile.setOutlineColor(sf::Color(25, 30, 28));
    tile.setOutlineThickness(-1.f);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            switch (tiles[y][x]) {
            case TILE_WALL:   tile.setFillColor(sf::Color(68, 73, 78)); break;
            case TILE_PATH:   tile.setFillColor(sf::Color(91, 80, 58)); break;
            case TILE_SPAWN:  tile.setFillColor(sf::Color(130, 45, 55)); break;
            case TILE_DEPLOY: tile.setFillColor(sf::Color(41, 125, 155)); break;
            default:          tile.setFillColor(sf::Color(44, 92, 58)); break;
            }
            tile.setPosition({x * TILE_SIZE, y * TILE_SIZE});
            window.draw(tile);
        }
    }
}