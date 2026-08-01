#include "Player.h"
#include "GameContext.h"
#include <cmath>
#include <iostream>
#include "TextureManager.h"


Player::Player()
    : Entity(400.f, 300.f, 100, 100), playerTexture(nullptr) {
    team = Team::Player;

    const float desiredSize = 80.f;

    std::string textureKey = "player";
    if (!TextureManager::getInstance().hasTexture(textureKey)) {
        std::cerr << "Failed to load Ash texture!" << std::endl;
        useFallback = true;
        fallbackShape.setFillColor(sf::Color::Blue);
        // Tính toán và thiết lập origin ở trung tâm ảnh
            sf::Vector2u size = playerTexture->getSize();
        this->sprite->setOrigin({ static_cast<float>(size.x) / 2.f, static_cast<float>(size.y) / 2.f });

        // Đặt vị trí ban đầu
        this->sprite->setPosition({ 400.f, 300.f });

        // >>> CHÈN CODE SCALE Ở ĐÂY <<<
        // Ví dụ muốn player hiển thị ở kích thước 100x50 pixel
        float desiredWidth = 100.f;
        float desiredHeight = 50.f;

        float scaleX = desiredWidth / static_cast<float>(size.x);
        float scaleY = desiredHeight / static_cast<float>(size.y);

        this->sprite->setScale(sf::Vector2f(scaleX, scaleY));
        fallbackShape.setSize(sf::Vector2f(desiredHeight, desiredHeight));
        fallbackShape.setPosition(sf::Vector2f(400.f, 300.f));
    } else {
        std::cout << "Ash texture loaded OK!" << std::endl;
        useFallback = false;
        //playerTexture = &textureManager.getTexture("Ash");
        playerTexture = &TextureManager::getInstance().getTexture(textureKey);

        // Khởi tạo con trỏ unique_ptr<sf::Sprite> truyền vào texture
        this->sprite = std::make_unique<sf::Sprite>(*playerTexture);


        // Tính toán và thiết lập origin ở trung tâm ảnh
        sf::Vector2u size = playerTexture->getSize();
        this->sprite->setOrigin({ static_cast<float>(size.x) / 2.f, static_cast<float>(size.y) / 2.f });

        // Đặt vị trí ban đầu
        this->sprite->setPosition({ 400.f, 300.f });

        // >>> CHÈN CODE SCALE Ở ĐÂY <<<
        // Ví dụ muốn player hiển thị ở kích thước 100x50 pixel
        float desiredWidth =80.f;
        float desiredHeight = 160.f;

        float scaleX = desiredWidth / static_cast<float>(size.x);
        float scaleY = desiredHeight / static_cast<float>(size.y);

        this->sprite->setScale(sf::Vector2f(scaleX, scaleY));
        fallbackShape.setSize(sf::Vector2f(desiredHeight, desiredHeight));

        // Đặt vị trí ban đầu và tốc độa
        this->sprite->setPosition({ 400.f, 300.f });
    }
}

void Player::handleInput() {
    sf::Vector2f movement(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) movement.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) movement.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) movement.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) movement.x += 1.f;

    if (movement.x != 0.f || movement.y != 0.f) {
        float length = std::sqrt(movement.x * movement.x + movement.y * movement.y);
        movement /= length;
    }
    setDirection(movement);

    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !isAttacking) {
        setIsAttacking(true);
    }
}

void Player::update(const GameContext& context) {
    sf::Vector2f dir = getDirection();
    if (dir.x != 0.f || dir.y != 0.f) {
        float newX = getX() + dir.x * speed * context.deltaTime;
        float newY = getY() + dir.y * speed * context.deltaTime;
        setPosition(newX, newY);
        //playerShape.setPosition(getPosition());
        // Cập nhật vị trí Sprite (Dùng dấu -> vì sprite là unique_ptr)
        if (sprite) {
            sprite->setPosition(getPosition());
        }
        fallbackShape.setPosition(getPosition());
    }
}

void Player::draw(sf::RenderWindow& window) {
    if (useFallback) {
        window.draw(fallbackShape);
    } else if(sprite) {
        window.draw(*sprite); // Giải băm unique_ptr khi vẽ
    }
}

sf::FloatRect Player::getCollisionBox() const {
    //if (useFallback) return fallbackShape.getGlobalBounds();
    //return playerShape.getGlobalBounds();
    if (useFallback || !sprite) return fallbackShape.getGlobalBounds();
    return sprite->getGlobalBounds();
}

sf::FloatRect Player::getHurtBox() const {
    if (useFallback || !sprite) return fallbackShape.getGlobalBounds();
    return sprite->getGlobalBounds();
}
