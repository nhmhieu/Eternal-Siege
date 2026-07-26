#include "TextureManager.h"
#include <iostream>

bool TextureManager::loadTexture(const std::string& name,
    const std::string& path)
{
    sf::Texture texture;

    // Thử đường dẫn gốc
    if (texture.loadFromFile(path)) {
        textures[name] = std::move(texture);
        return true;
    }
    
    // Nếu fail, thử các path khác (khi chạy từ thư mục build)
    std::string filename = path;
    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        filename = path.substr(lastSlash + 1);
    }
    
    // Thử "../assets/images/" (khi chạy từ out/build/x64-debug/)
    std::string altPath1 = "../assets/images/" + filename;
    if (texture.loadFromFile(altPath1)) {
        textures[name] = std::move(texture);
        return true;
    }
    
    // Thử "assets/images/" (khi chạy từ thư mục gốc)
    std::string altPath2 = "assets/images/" + filename;
    if (texture.loadFromFile(altPath2)) {
        textures[name] = std::move(texture);
        return true;
    }

    std::cout << "Failed to load texture: " << path << std::endl;
    return false;
}

sf::Texture& TextureManager::getTexture(const std::string& name)
{
    auto it = textures.find(name);
    if (it != textures.end()) {
        return it->second;
    }
    
    // Trả về texture rỗng (sprite sẽ không vẽ gì thay vì crash)
    static sf::Texture emptyTexture;
    std::cerr << "Texture '" << name << "' not found!" << std::endl;
    return emptyTexture;
}

