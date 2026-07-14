#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

constexpr float TILE_SIZE = 40.f;

class Map
{
public:
    Map(int width, int height);
    void handleMouseClick(int mouseX, int mouseY);
    void generate();
    void draw(sf::RenderWindow& window);

private:
    int width;
    int height;

    std::vector<std::vector<int>> tiles;
};