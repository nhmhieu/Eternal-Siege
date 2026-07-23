#include "TextureManager.h"
#include <iostream>

bool TextureManager::loadTexture(const std::string& name,
    const std::string& filename)
{
    sf::Texture texture;

    if (!texture.loadFromFile(filename))
    {
        std::cout << "Failed to load texture: "
            << filename << std::endl;
        return false;
    }

    textures[name] = std::move(texture);
    return true;
}

sf::Texture& TextureManager::getTexture(const std::string& name)
{
    return textures.at(name);
}

bool TextureManager::hasTexture(const std::string& name) const
{
    return textures.find(name) != textures.end();
}
