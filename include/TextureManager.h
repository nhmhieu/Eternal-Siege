#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <string>

class TextureManager
{
private:
    std::map<std::string, sf::Texture> textures;

public:
    // Load một texture từ file
    bool loadTexture(const std::string& name,
        const std::string& filename);

    // Lấy texture đã load
    sf::Texture& getTexture(const std::string& name);
};

