#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>

class TextureManager
{
private:
    TextureManager() = default;

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    std::unordered_map<std::string, sf::Texture> textures;

public:
    static TextureManager& getInstance();

    // Load một texture từ file
    bool loadTexture(const std::string& name,
        const std::string& filename);

    // Lấy texture đã load
    sf::Texture& getTexture(const std::string& name);

    // Kiểm tra texture đã tồn tại chưa
    bool hasTexture(const std::string& name) const;
};

