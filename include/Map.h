#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include <array>
#include <string_view>
#include <vector>

class TextureManager;

enum TileType {
    TILE_GRASS = 0,
    TILE_WALL,
    TILE_PATH,
    TILE_SPAWN,
    TILE_DEPLOY
};

std::array<std::string_view, 2> mapTextureKeys(TileType type);
std::size_t selectTileVariant(
    int row, int column, TileType type, std::size_t variantCount) noexcept;

class Map {
public:
    Map(int width, int height);

    void generate();
    void setTextureManager(TextureManager& manager);
    void draw(sf::RenderWindow& window) const;
    void handleMouseClick(int mouseX, int mouseY,
                          std::vector<sf::Vector2i>& selectedPositions,
                          int maxAllies);

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    bool isInside(sf::Vector2i cell) const;
    bool isSolid(sf::Vector2i cell) const;
    bool isWalkable(int x, int y) const;
    bool isWalkable(sf::Vector2i cell) const;
    TileType getTileType(int x, int y) const { return tiles[y][x]; }
    bool isWalkableWorld(sf::Vector2f center, float halfSize = 14.f) const;
    bool collidesWithSolid(const sf::FloatRect& bounds) const;
    sf::FloatRect getWorldBounds() const;
    sf::Vector2f resolveMovement(
        sf::Vector2f center,
        sf::Vector2f halfExtents,
        sf::Vector2f displacement
    ) const;
    std::optional<sf::Vector2f> findNearestValidPosition(
        sf::Vector2f preferred,
        sf::Vector2f halfExtents
    ) const;

    sf::Vector2i worldToGrid(sf::Vector2f world) const;
    sf::Vector2f gridToWorld(sf::Vector2i cell) const;
    sf::Vector2i nearestWalkable(sf::Vector2i cell) const;

    std::vector<sf::Vector2i> findPathBFS(
        sf::Vector2i start, sf::Vector2i goal) const;
    const std::vector<sf::Vector2i>& getEnemySpawnCells() const {
        return enemySpawnCells;
    }

private:
    void loadTileTextures();
    int width;
    int height;
    std::vector<std::vector<TileType>> tiles;
    std::vector<sf::Vector2i> enemySpawnCells;
    TextureManager* textureManager = nullptr;
    std::array<std::array<const sf::Texture*, 2>, 3> tileTextures{};
};
