#include "TextureManager.h"
#include <iostream>

TextureManager& TextureManager::getInstance()
{
    static TextureManager instance;
    return instance;
}

bool TextureManager::loadTexture(const std::string& name,
    const std::string& filename)
{
    sf::Texture texture;

    if (!texture.loadFromFile(filename))
    {
        std::cout << "[FAILED] Load texture: "
            << filename << std::endl;
        return false;
    }

    textures[name] = std::move(texture);

    std::cout << "[SUCCESS] Load texture: "
        << filename << std::endl;

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
