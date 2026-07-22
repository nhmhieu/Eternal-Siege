#include "TextureManager.h"
#include <iostream>

bool TextureManager::loadTexture(const std::string& name,
    const std::string& path)
{
    sf::Texture texture;

    if (!texture.loadFromFile(path))
    {
        std::cout << "Failed to load texture: "
            << path << std::endl;
        return false;
    }

    textures[name] = std::move(texture);
    return true;
}

sf::Texture& TextureManager::getTexture(const std::string& name)
{
    return textures.at(name);
}

