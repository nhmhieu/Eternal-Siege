#include "AssetLocator.h"
#include "TextureManager.h"
#include "KingdomAssets.h"
#include "IntroState.h"
#include "Effects.h"
#include "EndScreenView.h"

#include <array>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {
void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "Presentation test failed: " << message << '\n';
        std::exit(1);
    }
}
}

int main() {
    float introDuration = 0.f;
    for (float duration : IntroState::FRAME_DURATIONS) {
        require(duration > 0.f, "every intro frame has a positive duration");
        introDuration += duration;
    }
    require(introDuration == 18.f, "intro duration totals 18 seconds");
    require(AssetLocator::find("assets/fonts/Font.ttf").has_value(),
            "portable font path");
    require(!AssetLocator::find(
                "assets/images/intentionally_missing_for_test.png").has_value(),
            "missing asset returns null");

    TextureManager textures;
    require(textures.loadTexture(
                "SpiritStaff", "assets/images/SpiritStaff.png"),
            "Spirit Staff texture loads with its canonical key");
    const std::array<std::string, 5> names{{
        "PlayerMage", "Damian", "Evangeline", "Junior", "Lucas"
    }};
    for (const auto& name : names) {
        require(textures.loadTexture(
                    name, "assets/images/" + name + ".png"),
                "existing character texture loads");
        require(textures.findTexture(name) != nullptr,
                "loaded texture can be found");
        const sf::IntRect bounds = textures.getVisibleBounds(name);
        require(bounds.size.x > 0 && bounds.size.y > 0,
                "visible alpha bounds are non-empty");
    }

    require(!textures.loadTexture(
                "Missing", "assets/images/does_not_exist.png"),
            "missing texture falls back without throwing");
    require(textures.findTexture("Missing") == nullptr,
            "missing texture is not cached as a real texture");
    const auto hitsBefore=textures.getCacheHits();
    require(textures.loadTexture("PlayerMage","assets/images/PlayerMage.png"),"cached texture remains available");
    require(textures.getCacheHits()==hitsBefore+1,"second asset request is a cache hit");
    const auto kingdomMissesBefore=textures.getCacheMisses();require(preloadKingdomAssets(textures),"Kingdom manifest preloads");const auto kingdomMissesAfterFirst=textures.getCacheMisses();require(preloadKingdomAssets(textures),"Kingdom manifest remains available on second entry");require(textures.getCacheMisses()==kingdomMissesAfterFirst&&kingdomMissesAfterFirst>kingdomMissesBefore,"second Kingdom preload performs no texture loads");

    const sf::Texture* playerMage = textures.findTexture("PlayerMage");
    const sf::IntRect playerMageBounds =
        textures.getVisibleBounds("PlayerMage");
    require(playerMage &&
                (playerMageBounds.size.x <
                     static_cast<int>(playerMage->getSize().x) ||
                 playerMageBounds.size.y <
                     static_cast<int>(playerMage->getSize().y)),
            "transparent character canvas is cropped for presentation");

    const std::array<std::string, 3> monsterNames{{
        "NormalMonster", "EliteMonster", "Boss"
    }};
    for (const auto& name : monsterNames) {
        require(textures.loadTexture(
                    name, "assets/images/monsters/" + name + ".png"),
                "monster texture loads from canonical path");
        const sf::Texture* texture = textures.findTexture(name);
        const sf::IntRect bounds = textures.getVisibleBounds(name);
        require(texture && bounds.size.x > 0 && bounds.size.y > 0,
                "monster visible alpha bounds are non-empty");
        require(bounds.size.x < static_cast<int>(texture->getSize().x) ||
                    bounds.size.y < static_cast<int>(texture->getSize().y),
                "transparent monster canvas is cropped");
    }

    Effects effects;
    effects.spawnSlash({100.f, 100.f}, {1.f, 0.f}, false);
    require(effects.slashCount() == 1,
            "one slash event creates one slash effect");
    const std::size_t beforeShot = effects.activeEffectCount();
    effects.spawnShot({100.f, 100.f}, {1.f, 0.f}, true);
    require(effects.activeEffectCount() > beforeShot,
            "magic cast creates presentation effects");
    effects.update(2.f);
    require(effects.activeEffectCount() == 0,
            "expired effects are removed");

    EndScreenView endScreen(textures);
    endScreen.initialize(EndScreenTheme::Victory);
    require(endScreen.getTheme() == EndScreenTheme::Victory,
            "victory end-screen theme");
    require(endScreen.particleCount() > 0,
            "end screen creates ambient particles");
    require(endScreen.restartHit({640.f, 440.f}),
            "play-again button hitbox");
    require(endScreen.menuHit({640.f, 520.f}),
            "main-menu button hitbox");
    endScreen.initialize(EndScreenTheme::Defeat);
    require(endScreen.getTheme() == EndScreenTheme::Defeat,
            "defeat end-screen theme");

    std::cout << "Presentation asset tests passed\n";
    return 0;
}
