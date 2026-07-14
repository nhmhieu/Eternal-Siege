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
            tiles[y][x] = 0;
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
            if (tiles[y][x] == 1)
            {
                tile.setFillColor(sf::Color::Green);
            }
            else
            {
                tile.setFillColor(sf::Color::White);
            }
            tile.setPosition(sf::Vector2f(x * TILE_SIZE, y * TILE_SIZE));
            window.draw(tile);
        }
    }
}

void Map::handleMouseClick(int mouseX, int mouseY)
{
    int gridX = mouseX / static_cast<int>(TILE_SIZE);
    int gridY = mouseY / static_cast<int>(TILE_SIZE);

    if (gridX >= 0 && gridX < width && gridY >= 0 && gridY < height)
    {
        tiles[gridY][gridX] = (tiles[gridY][gridX] == 0) ? 1 : 0;
        std::cout << "Clicked Cell: (" << gridX << ", " << gridY << ")\n";
    }
}