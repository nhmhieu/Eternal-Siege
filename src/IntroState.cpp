#include "IntroState.h"

#include "AssetLocator.h"
#include "MenuState.h"

#include <array>

namespace {
const std::array<const char*, 4> STORY_FRAMES{{
    "ETERNAL SIEGE",
    "For centuries, the Eternal Gate protected the realm\n"
    "from the armies of the Abyss.",
    "Now the seal is breaking.\n"
    "The Spirit Warden and four guardians must hold the final stronghold.",
    "Defend the gate.\nSurvive four waves.\nDefeat the Abyssal Lord."
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
    storyText.emplace(font, "", 38);
    storyText->setFillColor(sf::Color(235, 225, 195));
    hintText.emplace(font, "Space / Enter: next    Escape: skip", 18);
    hintText->setFillColor(sf::Color(150, 150, 150));
    const auto hintBounds = hintText->getLocalBounds();
    hintText->setOrigin({hintBounds.position.x + hintBounds.size.x / 2.f,
                         hintBounds.position.y + hintBounds.size.y / 2.f});
    hintText->setPosition({640.f, 665.f});

    if (textureManager.loadTexture("Logo", "assets/images/ui/logo.png")) {
        if (const sf::Texture* logo = textureManager.findTexture("Logo")) {
            logoSprite.emplace(*logo);
            const sf::Vector2u size = logo->getSize();
            const float scale = std::min(600.f / static_cast<float>(size.x),
                                         225.f / static_cast<float>(size.y));
            logoSprite->setScale({scale, scale});
            logoSprite->setOrigin({static_cast<float>(size.x) / 2.f,
                                   static_cast<float>(size.y) / 2.f});
            logoSprite->setPosition({640.f, 250.f});
        }
    }
    showFrame();
}

void IntroState::onExit() {}

void IntroState::showFrame() {
    frameTimer = 0.f;
    if (!storyText) return;
    storyText->setString(STORY_FRAMES[static_cast<std::size_t>(frameIndex)]);
    storyText->setCharacterSize(frameIndex == 0 ? 72u : 38u);
    const sf::FloatRect bounds = storyText->getLocalBounds();
    storyText->setOrigin({bounds.position.x + bounds.size.x / 2.f,
                          bounds.position.y + bounds.size.y / 2.f});
    storyText->setPosition({640.f, frameIndex == 0 && logoSprite ? 455.f : 330.f});
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
        if (++frameIndex >= static_cast<int>(STORY_FRAMES.size())) finishIntro();
        else showFrame();
    }
}

void IntroState::update(float dt) {
    if (transitionRequested) return;
    frameTimer += dt;
    if (frameTimer < FRAME_DURATIONS[static_cast<std::size_t>(frameIndex)]) {
        return;
    }
    if (++frameIndex >= static_cast<int>(STORY_FRAMES.size())) finishIntro();
    else showFrame();
}

void IntroState::render(sf::RenderWindow& target) {
    target.clear(sf::Color(7, 8, 15));
    if (frameIndex == 0 && logoSprite) target.draw(*logoSprite);
    if (storyText) target.draw(*storyText);
    if (hintText) target.draw(*hintText);
}
