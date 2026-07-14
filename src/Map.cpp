#include "Map.h"
#include <iostream>

Map::Map(int width, int height)
{
    this->width = width;
    this->height = height;
    generate();
}

void Map::generate()
{
    tiles.resize(height);
    for (int y = 0; y < height; y++)
    {
        tiles[y].resize(width);
        for (int x = 0; x < width; x++)
        {
            tiles[y][x] = TITLE_EMPTY;
        }
    }
}

void Map::draw(sf::RenderWindow& window)
{
    sf::RectangleShape tile(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    tile.setOutlineColor(sf::Color::Black);
    tile.setOutlineThickness(-1.f);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if (tiles[y][x] == TITLE_OCCUPIED)
                tile.setFillColor(sf::Color::Cyan);
            else if (tiles[y][x] == TITLE_WALL)
                tile.setFillColor(sf::Color::Green);
            else
                tile.setFillColor(sf::Color::White);

            tile.setPosition(sf::Vector2f(x * TILE_SIZE, y * TILE_SIZE));
            window.draw(tile);
        }
    }
}

void Map::handleMouseClick(int mouseX, int mouseY, std::vector<sf::Vector2i>& selectedPositions, int maxAllies)
{
    int gridX = static_cast<int>(mouseX / TILE_SIZE);
    int gridY = static_cast<int>(mouseY / TILE_SIZE);

    if (gridX >= 0 && gridX < width && gridY >= 0 && gridY < height)
    {
        if (tiles[gridY][gridX] == TITLE_EMPTY && static_cast<int>(selectedPositions.size()) < maxAllies)
        {
            tiles[gridY][gridX] = TITLE_OCCUPIED;
            selectedPositions.push_back(sf::Vector2i(gridX, gridY));
            std::cout << "Dat tướng tai (" << gridX << ", " << gridY << ")" << std::endl;
        }
        else
        {
            std::cout << "O da co tướng hoac da du 4 tướng!" << std::endl;
        }
    }
}