#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

enum TileType {
    TILE_GRASS = 0,
    TILE_WALL,
    TILE_PATH,
    TILE_SPAWN,
    TILE_DEPLOY
};

class Map {
public:
    Map(int width, int height);

    void generate();
    void draw(sf::RenderWindow& window) const;
    void handleMouseClick(int mouseX, int mouseY,
                          std::vector<sf::Vector2i>& selectedPositions,
                          int maxAllies);

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    bool isInside(sf::Vector2i cell) const;
    bool isWalkable(int x, int y) const;
    bool isWalkable(sf::Vector2i cell) const;
    bool isWalkableWorld(sf::Vector2f center, float halfSize = 14.f) const;

    sf::Vector2i worldToGrid(sf::Vector2f world) const;
    sf::Vector2f gridToWorld(sf::Vector2i cell) const;
    sf::Vector2i nearestWalkable(sf::Vector2i cell) const;

    std::vector<sf::Vector2i> findPathBFS(
        sf::Vector2i start, sf::Vector2i goal) const;
    const std::vector<sf::Vector2i>& getEnemySpawnCells() const {
        return enemySpawnCells;
    }

private:
    int width;
    int height;
    std::vector<std::vector<TileType>> tiles;
    std::vector<sf::Vector2i> enemySpawnCells;
};