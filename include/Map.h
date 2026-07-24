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

constexpr int TITLE_EMPTY = 0;
constexpr int TITLE_WALL = 1;
constexpr int TITLE_OCCUPIED = 2;
constexpr float TILE_SIZE = 40.f;

class Map
{
public:
    Map(int width, int height);
    void handleMouseClick(int mouseX, int mouseY, std::vector<sf::Vector2i>& selectedPositions, int maxAllies);
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