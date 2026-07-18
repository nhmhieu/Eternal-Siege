#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>

class TextureManager
{
private:
    std::unordered_map<std::string, sf::Texture> textures;

public:
    // Load một texture từ file
    bool loadTexture(const std::string& name,
        const std::string& filename);

    // Lấy texture đã load
    sf::Texture& getTexture(const std::string& name);

    // Kiểm tra texture đã tồn tại chưa
    bool hasTexture(const std::string& name) const;
};

