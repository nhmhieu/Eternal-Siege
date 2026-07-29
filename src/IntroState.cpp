#include "IntroState.h"
#include "MenuState.h"
#include <iostream>
#include <algorithm> // Bổ sung để dùng std::clamp

IntroState::IntroState(
    StateMachine& machine,
    sf::RenderWindow& window,
    TextureManager& textureManager
)
    : stateMachine(machine),
      window(window),
      textureManager(textureManager) {
}

void IntroState::setupPositions() {
    float windowWidth = static_cast<float>(window.getSize().x);
    float windowHeight = static_cast<float>(window.getSize().y);

    // ===================================================
    // 1. NHÓM 1: CÙNG 1 DÒNG ("From nowhere of the universe")
    // ===================================================
    if (line1aText && line1bText) {
        float spacing = 12.f; // Khoảng cách giữa 2 đoạn chữ

        sf::FloatRect bounds1a = line1aText->getLocalBounds();
        sf::FloatRect bounds1b = line1bText->getLocalBounds();

        float totalWidth = bounds1a.size.x + spacing + bounds1b.size.x;
        float startX = (windowWidth - totalWidth) / 2.f;
        float centerY = windowHeight / 2.f;

        // --- Dòng 1a: "From nowhere" ---
        line1aText->setOrigin({ bounds1a.position.x, bounds1a.position.y + bounds1a.size.y / 2.f });
        line1aText->setPosition({ startX, centerY });

        // --- Dòng 1b: "of the universe" ---
        float startX_1b = startX + bounds1a.size.x + spacing;
        line1bText->setOrigin({ bounds1b.position.x, bounds1b.position.y + bounds1b.size.y / 2.f });
        line1bText->setPosition({ startX_1b, centerY });
    }

    // ===================================================
    // 2. NHÓM 2: 2 DÒNG TRÊN DƯỚI ("Created by" & "Ngo0Group")
    // ===================================================
    if (createdByText) {
        sf::FloatRect b2a = createdByText->getLocalBounds();
        createdByText->setOrigin({ b2a.position.x + b2a.size.x / 2.f, b2a.position.y + b2a.size.y / 2.f });
        createdByText->setPosition({ windowWidth / 2.f, windowHeight / 2.f - 30.f });
    }

    if (groupNameText) {
        sf::FloatRect b2b = groupNameText->getLocalBounds();
        groupNameText->setOrigin({ b2b.position.x + b2b.size.x / 2.f, b2b.position.y + b2b.size.y / 2.f });
        groupNameText->setPosition({ windowWidth / 2.f, windowHeight / 2.f + 35.f });
    }
}

void IntroState::onEnter() {
    std::cout << "--- DANG MO INTRO STATE ---" << std::endl;

    if (!font.openFromFile("assets/fonts/Font.ttf")) {
        std::cerr << "IntroState: Failed to load font!" << std::endl;
        stateMachine.changeState(std::make_unique<MenuState>(stateMachine, window, textureManager));
        return;
    }

    line1aText.emplace(font, "From nowhere", 32);
    line1bText.emplace(font, "of the universe", 32);
    createdByText.emplace(font, "Created by", 24);
    groupNameText.emplace(font, "Ngo0Group", 52);

    setupPositions();
}

void IntroState::onExit() {
    std::cout << "--- DANG ROI INTRO STATE ---" << std::endl;
}

void IntroState::handleEvent(const sf::Event& event) {
    if (event.is<sf::Event::KeyPressed>() || event.is<sf::Event::MouseButtonPressed>()) {
        stateMachine.changeState(std::make_unique<MenuState>(stateMachine, window, textureManager));
    }
}

void IntroState::update(float dt) {
    switch (currentPhase) {
    case TextPhase::Line1a_FadeIn:
        alpha1a += fadeSpeed * dt;
        if (alpha1a >= 255.f) {
            alpha1a = 255.f;
            currentPhase = TextPhase::Line1b_FadeIn;
        }
        break;

    case TextPhase::Line1b_FadeIn:
        alpha1b += fadeSpeed * dt;
        if (alpha1b >= 255.f) {
            alpha1b = 255.f;
            currentPhase = TextPhase::Line1_Hold;
            holdTimer = 0.f;
        }
        break;

    case TextPhase::Line1_Hold:
        holdTimer += dt;
        if (holdTimer >= holdDuration) {
            currentPhase = TextPhase::Line1_FadeOut;
        }
        break;

    case TextPhase::Line1_FadeOut:
        alpha1a -= fadeSpeed * dt;
        alpha1b -= fadeSpeed * dt;
        if (alpha1a <= 0.f) {
            alpha1a = 0.f;
            alpha1b = 0.f;
            currentPhase = TextPhase::Line2a_FadeIn;
        }
        break;

    case TextPhase::Line2a_FadeIn:
        alpha2a += fadeSpeed * dt;
        if (alpha2a >= 255.f) {
            alpha2a = 255.f;
            currentPhase = TextPhase::Line2b_FadeIn;
        }
        break;

    case TextPhase::Line2b_FadeIn:
        alpha2b += fadeSpeed * dt;
        if (alpha2b >= 255.f) {
            alpha2b = 255.f;
            currentPhase = TextPhase::Line2_Hold;
            holdTimer = 0.f;
        }
        break;

    case TextPhase::Line2_Hold:
        holdTimer += dt;
        if (holdTimer >= holdDuration) {
            currentPhase = TextPhase::Line2_FadeOut;
        }
        break;

    case TextPhase::Line2_FadeOut:
        alpha2a -= fadeSpeed * dt;
        alpha2b -= fadeSpeed * dt;
        if (alpha2a <= 0.f) {
            alpha2a = 0.f;
            alpha2b = 0.f;
            currentPhase = TextPhase::Finished;
        }
        break;

    case TextPhase::Finished:
        // Chuyển state và ngắt switch ngay lập tức
        stateMachine.changeState(std::make_unique<MenuState>(stateMachine, window, textureManager));
        return;
    }

    // Ép giá trị Alpha an toàn trong [0, 255] tránh chớp nháy màn hình
    auto getAlpha = [](float a) {
        return static_cast<std::uint8_t>(std::clamp(a, 0.f, 255.f));
        };

    if (line1aText) line1aText->setFillColor(sf::Color(255, 255, 255, getAlpha(alpha1a)));
    if (line1bText) line1bText->setFillColor(sf::Color(255, 255, 255, getAlpha(alpha1b)));
    if (createdByText) createdByText->setFillColor(sf::Color(200, 200, 200, getAlpha(alpha2a)));
    if (groupNameText) groupNameText->setFillColor(sf::Color(255, 215, 0, getAlpha(alpha2b)));
}

void IntroState::render(sf::RenderWindow& window) {
    window.clear(sf::Color::Black);

    if (currentPhase <= TextPhase::Line1_FadeOut) {
        if (line1aText) window.draw(*line1aText);
        if (line1bText) window.draw(*line1bText);
    }
    else if (currentPhase < TextPhase::Finished) {
        if (createdByText) window.draw(*createdByText);
        if (groupNameText) window.draw(*groupNameText);
    }
}