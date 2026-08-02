#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>

class TextureManager {
private:
    std::unordered_map<std::string, sf::Texture> textures;
    std::unordered_map<std::string, sf::IntRect> visibleBounds;

public:
    bool loadTexture(const std::string& name, const std::string& filename);
    sf::Texture& getTexture(const std::string& name);
    const sf::Texture* findTexture(const std::string& name) const;
    sf::IntRect getVisibleBounds(const std::string& name) const;
};
