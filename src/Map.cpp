#include "Map.h"
#include "Constants.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <queue>

using namespace GameConfig;

namespace {
int flatten(sf::Vector2i cell, int width) {
    return cell.y * width + cell.x;
}

sf::Vector2f depenetrate(
    const Map& map,
    sf::Vector2f center,
    sf::Vector2f halfExtents
) {
    const sf::FloatRect original{
        center - halfExtents,
        halfExtents * 2.f
    };
    if (!map.collidesWithSolid(original)) {
        return center;
    }

    constexpr float clearance = 0.001f;
    std::vector<float> xOffsets{0.f};
    std::vector<float> yOffsets{0.f};
    const sf::FloatRect world = map.getWorldBounds();
    const float left = original.position.x;
    const float top = original.position.y;
    const float right = left + original.size.x;
    const float bottom = top + original.size.y;
    const float worldRight = world.position.x + world.size.x;
    const float worldBottom = world.position.y + world.size.y;

    if (left < world.position.x) {
        xOffsets.push_back(world.position.x - left + clearance);
    }
    if (right > worldRight) {
        xOffsets.push_back(worldRight - right - clearance);
    }
    if (top < world.position.y) {
        yOffsets.push_back(world.position.y - top + clearance);
    }
    if (bottom > worldBottom) {
        yOffsets.push_back(worldBottom - bottom - clearance);
    }

    const int minX = std::max(
        0, static_cast<int>(std::floor(left / TILE_SIZE)));
    const int maxX = std::min(
        map.getWidth() - 1,
        static_cast<int>(std::floor(
            std::nextafter(
                right, -std::numeric_limits<float>::infinity()
            ) / TILE_SIZE
        )));
    const int minY = std::max(
        0, static_cast<int>(std::floor(top / TILE_SIZE)));
    const int maxY = std::min(
        map.getHeight() - 1,
        static_cast<int>(std::floor(
            std::nextafter(
                bottom, -std::numeric_limits<float>::infinity()
            ) / TILE_SIZE
        )));

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            if (!map.isSolid({x, y})) continue;

            const float tileLeft = static_cast<float>(x) * TILE_SIZE;
            const float tileTop = static_cast<float>(y) * TILE_SIZE;
            const float tileRight = tileLeft + TILE_SIZE;
            const float tileBottom = tileTop + TILE_SIZE;
            xOffsets.push_back(tileLeft - right - clearance);
            xOffsets.push_back(tileRight - left + clearance);
            yOffsets.push_back(tileTop - bottom - clearance);
            yOffsets.push_back(tileBottom - top + clearance);
        }
    }

    sf::Vector2f best = center;
    float bestDistanceSquared = std::numeric_limits<float>::infinity();
    constexpr float maxCorrection = TILE_SIZE * 2.f;
    const float maxCorrectionSquared = maxCorrection * maxCorrection;
    for (const float xOffset : xOffsets) {
        for (const float yOffset : yOffsets) {
            const float distanceSquared =
                xOffset * xOffset + yOffset * yOffset;
            if (distanceSquared <= 0.f ||
                distanceSquared >= bestDistanceSquared ||
                distanceSquared > maxCorrectionSquared) {
                continue;
            }

            const sf::Vector2f candidate =
                center + sf::Vector2f{xOffset, yOffset};
            if (!map.collidesWithSolid({
                    candidate - halfExtents,
                    halfExtents * 2.f
                })) {
                best = candidate;
                bestDistanceSquared = distanceSquared;
            }
        }
    }
    return best;
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

bool Map::isSolid(sf::Vector2i cell) const {
    return !isInside(cell) || tiles[cell.y][cell.x] == TILE_WALL;
}

bool Map::isWalkable(int x, int y) const {
    return isWalkable({x, y});
}

bool Map::isWalkable(sf::Vector2i cell) const {
    return !isSolid(cell);
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
    return !collidesWithSolid({
        center - sf::Vector2f(halfSize, halfSize),
        {halfSize * 2.f, halfSize * 2.f}
    });
}

sf::FloatRect Map::getWorldBounds() const {
    return {
        {0.f, 0.f},
        {
            static_cast<float>(width) * TILE_SIZE,
            static_cast<float>(height) * TILE_SIZE
        }
    };
}

bool Map::collidesWithSolid(const sf::FloatRect& bounds) const {
    if (bounds.size.x <= 0.f || bounds.size.y <= 0.f) {
        return true;
    }

    const sf::FloatRect world = getWorldBounds();
    const float right = bounds.position.x + bounds.size.x;
    const float bottom = bounds.position.y + bounds.size.y;
    const float worldRight = world.position.x + world.size.x;
    const float worldBottom = world.position.y + world.size.y;
    if (bounds.position.x < world.position.x ||
        bounds.position.y < world.position.y ||
        right > worldRight ||
        bottom > worldBottom) {
        return true;
    }

    // nextafter keeps a box ending exactly on a tile edge from claiming the
    // neighbouring tile. Only the covered tile range is inspected.
    const float insideRight = std::nextafter(
        right, -std::numeric_limits<float>::infinity());
    const float insideBottom = std::nextafter(
        bottom, -std::numeric_limits<float>::infinity());
    const int minX = static_cast<int>(std::floor(bounds.position.x / TILE_SIZE));
    const int maxX = static_cast<int>(std::floor(insideRight / TILE_SIZE));
    const int minY = static_cast<int>(std::floor(bounds.position.y / TILE_SIZE));
    const int maxY = static_cast<int>(std::floor(insideBottom / TILE_SIZE));

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            if (isSolid({x, y})) {
                return true;
            }
        }
    }
    return false;
}

sf::Vector2f Map::resolveMovement(
    sf::Vector2f center,
    sf::Vector2f halfExtents,
    sf::Vector2f displacement
) const {
    if (!std::isfinite(center.x) || !std::isfinite(center.y) ||
        !std::isfinite(displacement.x) || !std::isfinite(displacement.y)) {
        return center;
    }

    halfExtents.x = std::max(0.5f, halfExtents.x);
    halfExtents.y = std::max(0.5f, halfExtents.y);
    center = depenetrate(*this, center, halfExtents);
    constexpr float maxStep = TILE_SIZE * 0.25f;
    const float longestAxis =
        std::max(std::abs(displacement.x), std::abs(displacement.y));
    const int stepCount = std::clamp(
        static_cast<int>(std::ceil(longestAxis / maxStep)), 1, 4096);
    const sf::Vector2f step = displacement / static_cast<float>(stepCount);

    bool xBlocked = false;
    bool yBlocked = false;
    for (int index = 0; index < stepCount; ++index) {
        if (!xBlocked && std::abs(step.x) > 0.f) {
            const sf::Vector2f candidate{center.x + step.x, center.y};
            if (!collidesWithSolid({
                    candidate - halfExtents, halfExtents * 2.f})) {
                center.x = candidate.x;
            } else {
                xBlocked = true;
            }
        }

        if (!yBlocked && std::abs(step.y) > 0.f) {
            const sf::Vector2f candidate{center.x, center.y + step.y};
            if (!collidesWithSolid({
                    candidate - halfExtents, halfExtents * 2.f})) {
                center.y = candidate.y;
            } else {
                yBlocked = true;
            }
        }
    }
    return center;
}

std::optional<sf::Vector2f> Map::findNearestValidPosition(
    sf::Vector2f preferred,
    sf::Vector2f halfExtents
) const {
    sf::Vector2i start = worldToGrid(preferred);
    start.x = std::clamp(start.x, 0, width - 1);
    start.y = std::clamp(start.y, 0, height - 1);

    std::queue<sf::Vector2i> frontier;
    std::vector<bool> visited(static_cast<std::size_t>(width * height), false);
    frontier.push(start);
    visited[static_cast<std::size_t>(flatten(start, width))] = true;

    constexpr std::array<sf::Vector2i, 4> directions = {{
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}
    }};
    while (!frontier.empty()) {
        const sf::Vector2i cell = frontier.front();
        frontier.pop();
        const sf::Vector2f candidate = gridToWorld(cell);
        if (!collidesWithSolid({
                candidate - halfExtents, halfExtents * 2.f})) {
            return candidate;
        }

        for (const sf::Vector2i direction : directions) {
            const sf::Vector2i next = cell + direction;
            if (!isInside(next)) continue;
            const std::size_t nextIndex =
                static_cast<std::size_t>(flatten(next, width));
            if (visited[nextIndex]) continue;
            visited[nextIndex] = true;
            frontier.push(next);
        }
    }
    return std::nullopt;
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
