#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

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

private:
    int width;
    int height;

    std::vector<std::vector<int>> tiles;
};