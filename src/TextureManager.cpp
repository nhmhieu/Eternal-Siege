#include "TextureManager.h"

#include "AssetLocator.h"

#include <algorithm>
#include <iostream>
#include <limits>

namespace {
sf::IntRect findVisibleBounds(const sf::Texture& texture) {
    const sf::Image image = texture.copyToImage();
    const sf::Vector2u size = image.getSize();
    if (size.x == 0 || size.y == 0) return {};

    unsigned minX = size.x;
    unsigned minY = size.y;
    unsigned maxX = 0;
    unsigned maxY = 0;
    bool found = false;
    for (unsigned y = 0; y < size.y; ++y) {
        for (unsigned x = 0; x < size.x; ++x) {
            // Ignore near-transparent antialiasing noise at the canvas edge.
            if (image.getPixel({x, y}).a <= 8) continue;
            found = true;
            minX = std::min(minX, x);
            minY = std::min(minY, y);
            maxX = std::max(maxX, x);
            maxY = std::max(maxY, y);
        }
    }
    if (!found) {
        return {{0, 0}, {static_cast<int>(size.x), static_cast<int>(size.y)}};
    }
    constexpr unsigned padding = 8;
    minX = minX > padding ? minX - padding : 0;
    minY = minY > padding ? minY - padding : 0;
    maxX = std::min(size.x - 1, maxX + padding);
    maxY = std::min(size.y - 1, maxY + padding);
    return {{static_cast<int>(minX), static_cast<int>(minY)},
            {static_cast<int>(maxX - minX + 1),
             static_cast<int>(maxY - minY + 1)}};
}
}

bool TextureManager::loadTexture(const std::string& name,
                                 const std::string& filename) {
    if (textures.find(name) != textures.end()) {++cacheHits;return true;}
    ++cacheMisses;
    const auto path = AssetLocator::find(filename);
    if (!path) return false;

    sf::Texture texture;
    if (!texture.loadFromFile(*path)) {
        std::cerr << "Asset decode failed: " << path->generic_string() << '\n';
        return false;
    }
    visibleBounds[name] = findVisibleBounds(texture);
    textures.insert_or_assign(name, std::move(texture));
    return true;
}

sf::Texture& TextureManager::getTexture(const std::string& name) {
    const auto it = textures.find(name);
    if (it != textures.end()) return it->second;
    static sf::Texture emptyTexture;
    return emptyTexture;
}

const sf::Texture* TextureManager::findTexture(const std::string& name) const {
    const auto it = textures.find(name);
    return it == textures.end() ? nullptr : &it->second;
}

sf::IntRect TextureManager::getVisibleBounds(const std::string& name) const {
    const auto it = visibleBounds.find(name);
    return it == visibleBounds.end() ? sf::IntRect{} : it->second;
}
