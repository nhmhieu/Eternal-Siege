#include "Map.h"
#include "Constants.h"
#include "TextureManager.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <queue>
#include <random>
#include <cstdint>
#include <iostream>

using namespace GameConfig;

namespace {
std::size_t visualTileIndex(TileType type) {
    if (type == TILE_WALL) return 2;
    if (type == TILE_PATH || type == TILE_SPAWN) return 1;
    return 0;
}

int flatten(sf::Vector2i cell, int width) {
    return cell.y * width + cell.x;
}
}

std::array<std::string_view, 2> mapTextureKeys(TileType type) {
    static constexpr std::array<std::string_view, 2> grass{{
        "MapGrass01", "MapGrass02"}};
    static constexpr std::array<std::string_view, 2> path{{
        "MapPath01", "MapPath02"}};
    static constexpr std::array<std::string_view, 2> wall{{
        "MapWall01", "MapWall02"}};
    if (type == TILE_WALL) return wall;
    if (type == TILE_PATH || type == TILE_SPAWN) return path;
    return grass;
}

std::size_t selectTileVariant(int row, int column, TileType type,
                              std::size_t variantCount) noexcept {
    if (variantCount == 0) return 0;
    std::uint32_t hash = 0x9e3779b9u;
    hash ^= static_cast<std::uint32_t>(row) + 0x85ebca6bu +
            (hash << 6u) + (hash >> 2u);
    hash ^= static_cast<std::uint32_t>(column) + 0xc2b2ae35u +
            (hash << 6u) + (hash >> 2u);
    hash ^= static_cast<std::uint32_t>(visualTileIndex(type)) * 0x27d4eb2du;
    hash ^= hash >> 16u;
    hash *= 0x7feb352du;
    hash ^= hash >> 15u;
    return static_cast<std::size_t>(hash) % variantCount;
}

namespace {
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

void Map::setTextureManager(TextureManager& manager) {
    textureManager = &manager;
    loadTileTextures();
}

void Map::loadTileTextures() {
    if (!textureManager) return;
    static constexpr std::array<std::array<std::string_view, 2>, 3> paths{{
        {{"assets/images/map/GrassTile01.png",
          "assets/images/map/GrassTile02.png"}},
        {{"assets/images/map/PathTile01.png",
          "assets/images/map/PathTile02.png"}},
        {{"assets/images/map/WallTile01.png",
          "assets/images/map/WallTile02.png"}},
    }};
    const std::array<TileType, 3> types{{
        TILE_GRASS, TILE_PATH, TILE_WALL}};
    for (std::size_t kind = 0; kind < paths.size(); ++kind) {
        const auto keys = mapTextureKeys(types[kind]);
        for (std::size_t variant = 0; variant < 2; ++variant) {
            const std::string key(keys[variant]);
            const std::string path(paths[kind][variant]);
            if (!textureManager->loadTexture(key, path)) {
                std::cerr << "Missing map texture " << key
                          << ": " << path << '\n';
                tileTextures[kind][variant] = nullptr;
                continue;
            }
            tileTextures[kind][variant] = textureManager->findTexture(key);
        }
    }
}

void Map::generate() {
    constexpr int maxAttempts = 10;
    std::random_device rd;

    for (int attempt = 0; attempt < maxAttempts; ++attempt) {
        std::mt19937 gen(rd());
        tiles.assign(height, std::vector<TileType>(width, TILE_WALL));

        int x = width / 2;
        int y = height / 2;

        std::uniform_int_distribution<> dir(0, 3);

        const int totalSteps = width * height * 4;
        for (int i = 0; i < totalSteps; ++i) {
            if (i % 5 == 0) {
                tiles[y][x] = TILE_PATH;
            } else {
                tiles[y][x] = TILE_GRASS;
            }

            switch (dir(gen)) {
                case 0: ++x; break;
                case 1: --x; break;
                case 2: ++y; break;
                case 3: --y; break;
            }

            if (x < 1) x = 1;
            if (x > width - 2) x = width - 2;
            if (y < 1) y = 1;
            if (y > height - 2) y = height - 2;
        }

        sf::Vector2i startCell{width / 2, height / 2};
        if (tiles[startCell.y][startCell.x] == TILE_WALL) {
            startCell = nearestWalkable(startCell);
        }

        if (tiles[startCell.y][startCell.x] == TILE_WALL) {
            continue;
        }

        std::vector<sf::Vector2i> connectedComponent;
        std::vector<std::vector<bool>> visited(height, std::vector<bool>(width, false));
        std::queue<sf::Vector2i> q;

        q.push(startCell);
        visited[startCell.y][startCell.x] = true;

        while (!q.empty()) {
            const auto current = q.front();
            q.pop();
            connectedComponent.push_back(current);

            static constexpr std::array<sf::Vector2i, 4> dirs{{
                {1, 0}, {-1, 0}, {0, 1}, {0, -1}
            }};

            for (const auto& d : dirs) {
                const sf::Vector2i next{current.x + d.x, current.y + d.y};
                if (isInside(next) && !visited[next.y][next.x] && tiles[next.y][next.x] != TILE_WALL) {
                    visited[next.y][next.x] = true;
                    q.push(next);
                }
            }
        }

        if (connectedComponent.size() < 16) {
            continue;
        }

        const int centerX = width / 2;
        const int centerY = height / 2;

        std::array<sf::Vector2i, 4> candidates;
        std::array<float, 4> maxDistances{-1.f, -1.f, -1.f, -1.f};
        std::array<bool, 4> foundQuad{false, false, false, false};

        for (const auto& cell : connectedComponent) {
            int quad = 0;
            if (cell.x >= centerX && cell.y < centerY) quad = 1;
            else if (cell.x < centerX && cell.y >= centerY) quad = 2;
            else if (cell.x >= centerX && cell.y >= centerY) quad = 3;

            const float dist = std::hypot(cell.x - centerX, cell.y - centerY);
            if (dist > maxDistances[quad]) {
                maxDistances[quad] = dist;
                candidates[quad] = cell;
                foundQuad[quad] = true;
            }
        }

        int foundCount = 0;
        for (bool f : foundQuad) {
            if (f) ++foundCount;
        }

        if (foundCount < 4) {
            std::vector<sf::Vector2i> sortedComponent = connectedComponent;
            std::sort(sortedComponent.begin(), sortedComponent.end(), [&](sf::Vector2i a, sf::Vector2i b) {
                return std::hypot(a.x - centerX, a.y - centerY) > std::hypot(b.x - centerX, b.y - centerY);
            });
            if (sortedComponent.size() >= 4) {
                candidates[0] = sortedComponent[0];
                candidates[1] = sortedComponent[sortedComponent.size() / 4];
                candidates[2] = sortedComponent[sortedComponent.size() / 2];
                candidates[3] = sortedComponent[sortedComponent.size() * 3 / 4];
                foundCount = 4;
            }
        }

        if (foundCount == 4) {
            enemySpawnCells = {candidates[0], candidates[1], candidates[2], candidates[3]};
            for (const auto cell : enemySpawnCells) {
                tiles[cell.y][cell.x] = TILE_SPAWN;
            }
            return;
        }
    }

    // Fallback static border layout if all attempts fail
    tiles.assign(height, std::vector<TileType>(width, TILE_GRASS));
    for (int x = 0; x < width; ++x) {
        tiles.front()[x] = TILE_WALL;
        tiles.back()[x] = TILE_WALL;
    }
    for (int y = 0; y < height; ++y) {
        tiles[y].front() = TILE_WALL;
        tiles[y].back() = TILE_WALL;
    }
    enemySpawnCells = {{1, 1}, {width - 2, 1}, {1, height - 2}, {width - 2, height - 2}};
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
    tile.setOutlineThickness(0.f);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const TileType type = tiles[y][x];
            const std::size_t kind = visualTileIndex(type);
            const std::size_t variant = selectTileVariant(y, x, type, 2);
            const sf::Texture* texture = tileTextures[kind][variant];
            tile.setTexture(texture, true);
            if (texture) {
                tile.setFillColor(sf::Color::White);
            } else if (type == TILE_WALL) {
                tile.setFillColor(sf::Color(68, 73, 78));
            } else if (type == TILE_PATH || type == TILE_SPAWN) {
                tile.setFillColor(sf::Color(91, 80, 58));
            } else {
                tile.setFillColor(sf::Color(44, 92, 58));
            }
            tile.setPosition({x * TILE_SIZE, y * TILE_SIZE});
            window.draw(tile);
        }
    }
}
