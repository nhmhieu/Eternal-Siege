#include "IntroState.h"

#include "AssetLocator.h"
#include "MenuState.h"

#include <array>
#include <cmath>
#include <sstream>

namespace {
struct StoryFrameData {
    const char* heading;
    const char* subtitle;
    const char* body;
    const char* emphasis;
};

const std::array<StoryFrameData, 5> STORY_FRAMES{{
    {
        "ETERNAL SIEGE",
        "The Last Stand of Asterfall",
        "",
        ""
    },
    {
        "THE LAST BASTION",
        "",
        "For centuries, Asterfall Kingdom stood behind the Eternal Gate, the final seal between the living realm and the Abyss.",
        ""
    },
    {
        "THE SEAL IS BREAKING",
        "",
        "Beneath the capital, the ancient seal within the Ruined Catacombs has begun to fail. Creatures of the Abyss are gathering beyond the breach.",
        ""
    },
    {
        "THE SPIRIT WARDEN",
        "",
        "The Spirit Warden returns to Asterfall with four guardians. Together, they must enter the Catacombs and establish the kingdom's final defense.",
        ""
    },
    {
        "THE ETERNAL SIEGE",
        "",
        "Four waves stand between Asterfall and ruin.",
        "Survive the siege.   Protect the last bastion.   Defeat the Abyssal Lord."
    }
}};
}

IntroState::IntroState(StateMachine& machine, sf::RenderWindow& gameWindow,
                       TextureManager& textures, AudioManager& audio,
                       GameProgress& gameProgress)
    : stateMachine(machine), window(gameWindow), textureManager(textures),
      audioManager(audio), progress(gameProgress) {}

void IntroState::onEnter() {
    audioManager.playMusic("assets/audio/music/menu_theme.ogg");
    const auto fontPath = AssetLocator::find("assets/fonts/Font.ttf");
    if (!fontPath || !font.openFromFile(*fontPath)) {
        finishIntro();
        return;
    }

    headingText.emplace(font, "", 36);
    subtitleText.emplace(font, "", 22);
    bodyText.emplace(font, "", 23);
    emphasisText.emplace(font, "", 21);

    hintText.emplace(font, "ENTER / SPACE  Continue        ESC  Skip", 16);
    hintText->setFillColor(sf::Color(150, 155, 165));
    const auto hintBounds = hintText->getLocalBounds();
    hintText->setOrigin({hintBounds.position.x + hintBounds.size.x / 2.f,
                         hintBounds.position.y + hintBounds.size.y / 2.f});
    hintText->setPosition({640.f, 672.f});

    if (textureManager.loadTexture("Logo", "assets/images/ui/logo.png")) {
        if (const sf::Texture* logo = textureManager.findTexture("Logo")) {
            logoSprite.emplace(*logo);
            const sf::Vector2u size = logo->getSize();
            const float scale = std::min(500.f / static_cast<float>(size.x),
                                         180.f / static_cast<float>(size.y));
            logoSprite->setScale({scale, scale});
            logoSprite->setOrigin({static_cast<float>(size.x) / 2.f,
                                   static_cast<float>(size.y) / 2.f});
            logoSprite->setPosition({640.f, 220.f});
        }
    }
    showFrame(0);
}

void IntroState::onExit() {}

std::string IntroState::wrapText(const sf::Font& f, const std::string& str, unsigned int characterSize, float maxWidth) {
    if (str.empty()) return "";
    std::string result;
    std::string currentLine;
    std::istringstream words(str);
    std::string word;
    sf::Text tempText(f, "", characterSize);

    while (words >> word) {
        std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
        tempText.setString(testLine);
        if (tempText.getLocalBounds().size.x > maxWidth && !currentLine.empty()) {
            result += currentLine + "\n";
            currentLine = word;
        } else {
            currentLine = testLine;
        }
    }
    if (!currentLine.empty()) {
        result += currentLine;
    }
    return result;
}

void IntroState::showFrame(int targetFrame) {
    frameIndex = targetFrame;
    frameTimer = 0.f;
    fadeAlpha = 1.f;
    pendingFrameIndex = -1;

    const auto& data = STORY_FRAMES[static_cast<std::size_t>(frameIndex)];

    if (headingText) {
        headingText->setString(data.heading);
        headingText->setCharacterSize(frameIndex == 0 ? 46u : 36u);
        headingText->setFillColor(sf::Color(245, 215, 130));
        const sf::FloatRect b = headingText->getLocalBounds();
        headingText->setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});
        headingText->setPosition({640.f, frameIndex == 0 && logoSprite ? 350.f : 210.f});
    }

    if (subtitleText) {
        if (data.subtitle && data.subtitle[0] != '\0') {
            subtitleText->setString(data.subtitle);
            subtitleText->setFillColor(sf::Color(185, 215, 225));
            const sf::FloatRect b = subtitleText->getLocalBounds();
            subtitleText->setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});
            subtitleText->setPosition({640.f, 400.f});
        } else {
            subtitleText->setString("");
        }
    }

    if (bodyText) {
        if (data.body && data.body[0] != '\0') {
            std::string wrapped = wrapText(font, data.body, 23, 760.f);
            bodyText->setString(wrapped);
            bodyText->setFillColor(sf::Color(230, 225, 210));
            const sf::FloatRect b = bodyText->getLocalBounds();
            bodyText->setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});
            bodyText->setPosition({640.f, 320.f});
        } else {
            bodyText->setString("");
        }
    }

    if (emphasisText) {
        if (data.emphasis && data.emphasis[0] != '\0') {
            emphasisText->setString(data.emphasis);
            emphasisText->setFillColor(sf::Color(255, 215, 80));
            const sf::FloatRect b = emphasisText->getLocalBounds();
            emphasisText->setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});
            emphasisText->setPosition({640.f, 435.f});
        } else {
            emphasisText->setString("");
        }
    }
}

void IntroState::finishIntro() {
    if (transitionRequested) return;
    transitionRequested = true;
    stateMachine.changeState(std::make_unique<MenuState>(
        stateMachine, window, textureManager, audioManager, progress));
}

void IntroState::handleEvent(const sf::Event& event) {
    if (const auto* released = event.getIf<sf::Event::KeyReleased>()) {
        if (released->code == sf::Keyboard::Key::Space ||
            released->code == sf::Keyboard::Key::Enter) {
            advanceKeyHeld = false;
        }
        return;
    }
    const auto* key = event.getIf<sf::Event::KeyPressed>();
    if (!key) return;
    if (key->code == sf::Keyboard::Key::Escape) {
        finishIntro();
    } else if (key->code == sf::Keyboard::Key::Space ||
               key->code == sf::Keyboard::Key::Enter) {
        if (advanceKeyHeld) return;
        advanceKeyHeld = true;
        if (frameIndex + 1 >= static_cast<int>(STORY_FRAMES.size())) {
            finishIntro();
        } else {
            showFrame(frameIndex + 1);
        }
    }
}

void IntroState::update(float dt) {
    if (transitionRequested) return;

    frameTimer += dt;
    if (frameTimer >= FRAME_DURATIONS[static_cast<std::size_t>(frameIndex)]) {
        if (frameIndex + 1 >= static_cast<int>(STORY_FRAMES.size())) {
            finishIntro();
        } else {
            showFrame(frameIndex + 1);
        }
    }
}

void IntroState::render(sf::RenderWindow& target) {
    target.clear(sf::Color(7, 10, 18));

    // Story panel card
    sf::RectangleShape card({860.f, 440.f});
    card.setOrigin({430.f, 220.f});
    card.setPosition({640.f, 360.f});
    card.setFillColor(sf::Color(10, 18, 28, 220));
    card.setOutlineColor(sf::Color(180, 145, 75, 140));
    card.setOutlineThickness(1.5f);
    target.draw(card);

    // Decorative inner gold line under heading
    sf::RectangleShape line({280.f, 2.f});
    line.setOrigin({140.f, 1.f});
    line.setPosition({640.f, frameIndex == 0 && logoSprite ? 382.f : 248.f});
    line.setFillColor(sf::Color(218, 175, 85, 180));
    target.draw(line);

    if (frameIndex == 0 && logoSprite) target.draw(*logoSprite);
    if (headingText) target.draw(*headingText);
    if (subtitleText && subtitleText->getString().getSize() > 0) target.draw(*subtitleText);
    if (bodyText && bodyText->getString().getSize() > 0) target.draw(*bodyText);
    if (emphasisText && emphasisText->getString().getSize() > 0) target.draw(*emphasisText);

    // 5 Progress Dots at bottom
    for (int i = 0; i < 5; ++i) {
        const float x = 640.f + static_cast<float>(i - 2) * 26.f;
        const float y = 625.f;
        const bool active = (i == frameIndex);
        const float r = active ? 5.5f : 3.5f;
        sf::CircleShape dot(r);
        dot.setOrigin({r, r});
        dot.setPosition({x, y});
        dot.setFillColor(active ? sf::Color(255, 215, 80, 245) : sf::Color(90, 95, 105, 180));
        target.draw(dot);
    }

    if (hintText) target.draw(*hintText);
}
