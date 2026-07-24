#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
enum TileType
{
    TILE_GRASS = 0,
    TILE_WALL,
    TILE_PATH,
    TILE_SPAWN
};

constexpr float TILE_SIZE = 40.f;

class Map
{
public:
    Map(int width, int height);
    void handleMouseClick(int mouseX, int mouseY);
    void generate();
    void draw(sf::RenderWindow& window);

const std::vector<sf::Vector2i>& getSelectedPositions() const;
bool canStart() const;

private:
    int width;
    int height;

    std::vector<std::vector<int>> tiles;
    int spawnCount = 0;

    std::vector<sf::Vector2i> selectedPositions;
};