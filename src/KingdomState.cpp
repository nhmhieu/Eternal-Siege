#include "KingdomState.h"
#include "LevelSelectState.h"
#include "MenuState.h"
#include "AssetLocator.h"
#include "KingdomAssets.h"
#include <algorithm>
#include <cmath>

KingdomState::KingdomState(
    StateMachine& m,
    sf::RenderWindow& w,
    TextureManager& t,
    AudioManager& a,
    GameProgress& p,
    bool back)
    : machine(m), window(w), textures(t), audio(a), progress(p), gate(p.castleGateOpen), returning(back)
{
}

void KingdomState::onEnter() {
    position = returning ? KingdomMap::RETURN_SPAWN : KingdomMap::SPAWN;
    transitioning = false;
    transitionLatch.reset();
    fade = returning ? 0.f : 1.f;
    entryRevealTimer = returning ? 2.5f : 0.f;
    elapsed = 0.f;
    promptAlpha = 0.f;
    titleBannerTimer = 0.f;
    titleBannerAlpha = 0.f;

    const auto path = AssetLocator::find("assets/fonts/Font.ttf");
    fontLoaded = path && font.openFromFile(*path);
    audio.playMusic("assets/audio/music/menu_theme.ogg");

    preloadKingdomAssets(textures);
    renderer = std::make_unique<KingdomRenderer>(textures);
    compositor = std::make_unique<KingdomCompositor>();
    compositor->ensure({1280, 720});

    avatar = std::make_unique<Player>(textures);
    if (const auto* s = textures.findTexture("PlayerMageWalk")) {
        avatar->useWalkSpriteSheet(*s);
    }
    avatar->setPosition(position);

    npcs.clear();
    for (const auto& definition : kingdomNpcRoutes()) {
        const char* key = (definition.role == KingdomNpcRole::Guard) ? "GuardWalk" : "CitizenWalk";
        if (const auto* t = textures.findTexture(key)) {
            npcs.push_back(std::make_unique<KingdomNpc>(definition.role, *t, definition.route));
        }
    }

    uiView = window.getDefaultView();
    worldView = sf::View(sf::FloatRect{{0, 0}, {1280, 720}});
    cameraCenter = position;
    cameraCenter.x = std::clamp(cameraCenter.x, 640.f, 1032.f);
    cameraCenter.y = std::clamp(cameraCenter.y, 360.f, 581.f);
    worldView.setCenter(cameraCenter);
}

void KingdomState::onExit() {
}

void KingdomState::handleEvent(const sf::Event& e) {
    if (transitioning) {
        return;
    }

    if (entryRevealTimer < 2.5f) {
        if (const auto* k = e.getIf<sf::Event::KeyPressed>()) {
            if (k->code == sf::Keyboard::Key::Space || k->code == sf::Keyboard::Key::Enter) {
                entryRevealTimer = 2.5f;
                fade = 0.f;
                return;
            }
        }
    }

    if (const auto* k = e.getIf<sf::Event::KeyPressed>()) {
        if (k->code == sf::Keyboard::Key::Escape) {
            machine.changeState(std::make_unique<MenuState>(machine, window, textures, audio, progress));
            return;
        }

        if (k->code == sf::Keyboard::Key::V) {
            navigationActive = !navigationActive;
            audio.playSound("ui_click");
            return;
        }

        if (k->code == sf::Keyboard::Key::F3) {
            debugCollision = !debugCollision;
            return;
        }
        if (k->code == sf::Keyboard::Key::F4) {
            dayNight.advancePhase();
            return;
        }
        if (k->code == sf::Keyboard::Key::F5) {
            environment.advanceWeather();
            return;
        }
        if (k->code == sf::Keyboard::Key::F6) {
            debugPerformance = !debugPerformance;
            return;
        }

        if (k->code == sf::Keyboard::Key::E) {
            if (gate.tryOpen(map.canInteractWithGate(position))) {
                audio.playSound("ui_click");
                return;
            }
            if (gate.getState() == GateState::Open && map.canInteract(position) && transitionLatch.request()) {
                transitioning = true;
                audio.playSound("ui_click");
            }
        }
    }
}

void KingdomState::update(float dt) {
    dt = std::min(dt, 1.f / 30.f);
    lastFrameDt = dt;
    elapsed += dt;
    dayNight.update(dt);
    environment.update(dt);

    if (entryRevealTimer < 2.5f) {
        entryRevealTimer += dt;
        if (entryRevealTimer < 0.3f) {
            fade = 1.0f;
        } else {
            const float progress = std::clamp((entryRevealTimer - 0.3f) / 2.2f, 0.f, 1.f);
            fade = 1.0f - (progress * progress * (3.f - 2.f * progress));
        }
    }

    if (transitioning) {
        fade = std::min(1.f, fade + dt / .32f);
        if (fade >= 1) {
            machine.changeState(std::make_unique<LevelSelectState>(machine, window, textures, audio, progress));
        }
        return;
    }

    gate.update(dt);
    if (gate.getState() == GateState::Open) {
        progress.castleGateOpen = true;
    }
    if (entryRevealTimer >= 2.5f && !transitioning) {
        fade = 0.f;
    }

    // Input & movement logic
    sf::Vector2f input;
    if (entryRevealTimer >= 0.8f) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) { input.y--; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) { input.y++; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) { input.x--; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) { input.x++; }
    }

    const float len = std::hypot(input.x, input.y);
    const auto before = position;
    if (len > 0) {
        input /= len;
        facing = input;
        std::vector<sf::Vector2f> actorFeet;
        actorFeet.reserve(npcs.size());
        for (const auto& n : npcs) {
            actorFeet.push_back(n->getFootPosition());
        }
        position = map.resolveMovementWithActors(position, input * 230.f * dt, actorFeet, 14, 12, !gate.blocksPassage());
    }

    const auto actual = position - before;
    if (avatar) {
        avatar->setPosition(position);
        avatar->updateNonCombatPresentation(dt, actual);
    }

    // NPC schedule & movement updates
    for (auto& n : npcs) {
        const auto activity = NpcScheduleSystem::activity(n->getRole(), dayNight.phase());
        n->setSchedulePaused(activity == NpcActivity::Rest);
        n->update(dt, map);
    }

    for (std::size_t i = 0; i < npcs.size(); ++i) {
        npcs[i]->avoid(position, map, 28);
        for (std::size_t j = 0; j < i; ++j) {
            npcs[i]->avoid(npcs[j]->getPosition(), map, 24);
        }
    }

    // Lighting & presentation
    const sf::Vector2f lights[] = {{84, 875}, {430, 410}, {845, 520}, {1300, 205}};
    const float night = dayNight.nightStrength();
    auto tint = [&](sf::Vector2f p) {
        float glow = 0;
        for (auto l : lights) {
            glow = std::max(glow, std::clamp(1.f - std::hypot(p.x - l.x, p.y - l.y) / 145.f, 0.f, 1.f));
        }
        return sf::Color(
            std::uint8_t(225 - 50 * night + 45 * glow * night),
            std::uint8_t(228 - 48 * night + 32 * glow * night),
            std::uint8_t(232 - 22 * night)
        );
    };

    if (avatar) {
        avatar->setPresentationTint(tint(position));
    }
    for (auto& n : npcs) {
        n->setTint(tint(n->getPosition()));
    }

    // Smooth responsive camera follow without dead-zone
    auto target = position;
    cameraCenter += (target - cameraCenter) * (1.f - std::exp(-12.f * dt));
    cameraCenter.x = std::clamp(cameraCenter.x, 640.f, 1032.f);
    cameraCenter.y = std::clamp(cameraCenter.y, 360.f, 581.f);
    worldView.setCenter(cameraCenter);

    const bool prompt = (gate.getState() == GateState::Closed && map.canInteractWithGate(position)) ||
                        (gate.getState() == GateState::Open && map.canInteract(position));
    promptAlpha += std::clamp((prompt ? 1.f : 0.f) - promptAlpha, -dt * 5, dt * 5);
}

void KingdomState::drawText(sf::RenderWindow& w, const std::string& s, sf::Vector2f p, unsigned n, sf::Color c, bool centered) const {
    if (fontLoaded) {
        sf::Text t(font, s, n);
        t.setFillColor(c);
        if (centered) {
            const auto b = t.getLocalBounds();
            t.setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});
            t.setPosition(p);
        } else {
            t.setPosition(p);
        }
        w.draw(t);
    }
}

void KingdomState::render(sf::RenderWindow& w) {
    if (!compositor) {
        return;
    }

    sceneTarget = &compositor->begin(worldView);
    if (renderer) {
        renderer->drawBase(*sceneTarget);
        renderer->drawWater(*sceneTarget, elapsed, dayNight.nightStrength());
    }

    const float open = gate.openProgress();
    const float eased = 1 - std::pow(1 - open, 3);
    if (const auto* door = textures.findTexture("CastleGate")) {
        const auto size = door->getSize();
        const int half = int(size.x / 2);
        sf::Sprite l(*door, {{0, 0}, {half, int(size.y)}});
        sf::Sprite r(*door, {{half, 0}, {int(size.x) - half, int(size.y)}});
        const float scale = 112.f / size.x;
        l.setScale({scale, scale});
        r.setScale({scale, scale});
        l.setPosition({164 - 55 * eased, 748});
        r.setPosition({220 + 55 * eased, 748});
        sceneTarget->draw(l);
        sceneTarget->draw(r);
    }

    if (renderer) {
        renderer->drawOccluder(*sceneTarget, KingdomRenderer::BRIDGE_FAR_INDEX);
    }

    struct Item {
        float key;
        int kind;
        std::size_t index;
    };

    std::vector<Item> items{{position.y, 0, 0}};
    for (std::size_t i = 0; i < npcs.size(); ++i) {
        items.push_back({npcs[i]->getSortKey(), 1, i});
    }

    if (renderer) {
        for (std::size_t i = 0; i < KingdomRenderer::OCCLUDER_COUNT; ++i) {
            if (i != KingdomRenderer::BRIDGE_FAR_INDEX && i != KingdomRenderer::BRIDGE_NEAR_INDEX) {
                items.push_back({renderer->occluderSortKey(i), 2, i});
            }
        }
    }

    std::stable_sort(items.begin(), items.end(), [](auto& a, auto& b) {
        return a.key < b.key;
    });

    auto shadow = [&](sf::Vector2f p, float scale) {
        if (map.isWater(p)) {
            return;
        }
        if (const auto* t = textures.findTexture("KingdomContactShadow")) {
            sf::Sprite s(*t);
            s.setOrigin({64, 32});
            s.setPosition({p.x, p.y + 3.f});
            s.setScale({scale * 0.95f, scale * 0.58f});
            s.setColor({255, 255, 255, std::uint8_t(210 - 55 * dayNight.nightStrength())});
            sceneTarget->draw(s);
        }
    };

    for (auto& i : items) {
        if (i.kind == 0 && avatar) {
            shadow(position, .62f);
            avatar->draw(*sceneTarget);
        } else if (i.kind == 1) {
            shadow(npcs[i.index]->getPosition(), .48f);
            npcs[i.index]->draw(*sceneTarget);
        } else if (renderer) {
            renderer->drawOccluder(*sceneTarget, i.index);
        }
    }

    if (renderer) {
        renderer->drawOccluder(*sceneTarget, KingdomRenderer::BRIDGE_NEAR_INDEX);
    }

    // World-Space Objective Marker for Ruined Catacombs when gate is open
    if (gate.getState() == GateState::Open) {
        const float bobY = std::sin(elapsed * 3.5f) * 3.f;
        const sf::Vector2f markerPos = KingdomMap::CAVE_CENTER + sf::Vector2f(0.f, -28.f + bobY);

        sf::CircleShape glowRing(18.f);
        glowRing.setOrigin({18.f, 18.f});
        glowRing.setPosition(markerPos);
        glowRing.setFillColor(sf::Color(255, 205, 50, 45));
        sceneTarget->draw(glowRing, sf::BlendAdd);

        sf::ConvexShape diamond(4);
        diamond.setPoint(0, {0.f, -12.f});
        diamond.setPoint(1, {10.f, 0.f});
        diamond.setPoint(2, {0.f, 12.f});
        diamond.setPoint(3, {-10.f, 0.f});
        diamond.setPosition(markerPos);
        diamond.setFillColor(sf::Color(255, 215, 70, 245));
        diamond.setOutlineColor(sf::Color(20, 30, 40, 220));
        diamond.setOutlineThickness(1.5f);
        sceneTarget->draw(diamond);
    }

    const sf::Vector2f targetPos = (gate.getState() == GateState::Closed) ? KingdomMap::GATE_CENTER : KingdomMap::CAVE_CENTER;

    if (navigationActive) {
        // Find closest waypoint on GOLDEN_ROUTE to player position
        std::size_t startIndex = 0;
        float minDistance = 1e9f;
        for (std::size_t i = 0; i < KingdomMap::GOLDEN_ROUTE.size(); ++i) {
            const float d = std::hypot(position.x - KingdomMap::GOLDEN_ROUTE[i].x, position.y - KingdomMap::GOLDEN_ROUTE[i].y);
            if (d < minDistance) {
                minDistance = d;
                startIndex = i;
            }
        }

        std::size_t targetIndex = KingdomMap::GOLDEN_ROUTE.size() - 1;
        if (gate.getState() == GateState::Closed) {
            targetIndex = 1;
        }

        // Build active path from player position -> forward route -> targetPos
        std::vector<sf::Vector2f> activePath;
        activePath.push_back(position);
        for (std::size_t i = startIndex; i <= targetIndex && i < KingdomMap::GOLDEN_ROUTE.size(); ++i) {
            activePath.push_back(KingdomMap::GOLDEN_ROUTE[i]);
        }
        if (activePath.empty() || activePath.back() != targetPos) {
            activePath.push_back(targetPos);
        }

        // Sample points along path at regular intervals (~70px)
        std::vector<sf::Vector2f> sampledPoints;
        constexpr float stepDistance = 70.f;
        for (std::size_t i = 0; i + 1 < activePath.size(); ++i) {
            sf::Vector2f p1 = activePath[i];
            sf::Vector2f p2 = activePath[i + 1];
            sf::Vector2f dir = p2 - p1;
            float len = std::hypot(dir.x, dir.y);
            if (len > 0.1f) {
                int steps = std::max(1, static_cast<int>(len / stepDistance));
                for (int s = 0; s < steps; ++s) {
                    float t = static_cast<float>(s) / static_cast<float>(steps);
                    sampledPoints.push_back(p1 + dir * t);
                }
            }
        }
        sampledPoints.push_back(targetPos);

        // Render golden trail with pulse animation
        for (std::size_t i = 0; i < sampledPoints.size(); ++i) {
            const sf::Vector2f pt = sampledPoints[i];
            const float pulse = 0.75f + 0.25f * std::sin(elapsed * 5.f - static_cast<float>(i) * 0.35f);

            sf::CircleShape glow(10.f * pulse);
            glow.setOrigin({10.f * pulse, 10.f * pulse});
            glow.setPosition(pt);
            glow.setFillColor(sf::Color(255, 200, 50, static_cast<std::uint8_t>(50.f * pulse)));
            sceneTarget->draw(glow, sf::BlendAdd);

            sf::CircleShape dot(5.f);
            dot.setOrigin({5.f, 5.f});
            dot.setPosition(pt);
            dot.setFillColor(sf::Color(255, 210, 60, static_cast<std::uint8_t>(220.f * pulse)));
            sceneTarget->draw(dot);
        }

        // Target Waypoint Beacon
        const float beaconPulse = 0.85f + 0.15f * std::sin(elapsed * 4.5f);
        sf::CircleShape beaconGlow(24.f * beaconPulse);
        beaconGlow.setOrigin({24.f * beaconPulse, 24.f * beaconPulse});
        beaconGlow.setPosition(targetPos);
        beaconGlow.setFillColor(sf::Color(255, 210, 60, 55));
        sceneTarget->draw(beaconGlow, sf::BlendAdd);

        sf::ConvexShape diamond(4);
        diamond.setPoint(0, {0.f, -10.f});
        diamond.setPoint(1, {8.f, 0.f});
        diamond.setPoint(2, {0.f, 10.f});
        diamond.setPoint(3, {-8.f, 0.f});
        diamond.setPosition(targetPos);
        diamond.setFillColor(sf::Color(255, 225, 90, 245));
        diamond.setOutlineColor(sf::Color(15, 25, 35, 220));
        diamond.setOutlineThickness(1.5f);
        sceneTarget->draw(diamond);
    }

    if (environment.getWeather() == KingdomWeather::LightRain) {
        for (int i = 0; i < 18; ++i) {
            sf::Vector2f p{
                std::fmod(i * 137.f + elapsed * 41.f, KingdomMap::WORLD_SIZE.x),
                std::fmod(i * 83.f + elapsed * 27.f, KingdomMap::WORLD_SIZE.y)
            };
            if (map.isWater(p)) {
                sf::CircleShape ripple(5);
                ripple.setOrigin({5, 5});
                ripple.setScale({1, .35f});
                ripple.setPosition(p);
                ripple.setFillColor(sf::Color::Transparent);
                ripple.setOutlineThickness(1);
                ripple.setOutlineColor({190, 225, 235, 75});
                sceneTarget->draw(ripple);
            }
        }
    }

    compositor->applyAmbient(dayNight.ambientOverlay());

    if (dayNight.nightStrength() > .01f) {
        for (std::size_t i = 0; i < 4; ++i) {
            const sf::Vector2f p = std::array<sf::Vector2f, 4>{{{84, 875}, {430, 410}, {845, 520}, {1300, 205}}}[i];
            sf::CircleShape glow(110);
            glow.setOrigin({110, 110});
            glow.setPosition(p);
            const float flicker = .92f + .08f * std::sin(elapsed * 3.1f + i * 2.7f);
            glow.setFillColor({255, 175, 70, std::uint8_t(34 * dayNight.nightStrength() * flicker)});
            sceneTarget->draw(glow, sf::BlendAdd);
        }
    }

    if (debugCollision) {
        auto rectangle = [&](sf::FloatRect r, sf::Color fill, sf::Color outline) {
            sf::RectangleShape q(r.size);
            q.setPosition(r.position);
            q.setFillColor(fill);
            q.setOutlineColor(outline);
            q.setOutlineThickness(2);
            sceneTarget->draw(q);
        };

        for (const auto& r : map.getWalkableRegions()) {
            rectangle(r.bounds, {45, 210, 95, 28}, {75, 235, 125, 105});
        }
        for (const auto& w : map.getWaterZones()) {
            rectangle(w, {25, 90, 235, 45}, {55, 155, 255, 180});
        }
        for (const auto& f : map.getSolidFootprints()) {
            rectangle(f.bounds, {235, 45, 45, 62}, {255, 95, 70, 215});
        }

        const sf::Vector2f a{1030, 655}, b{1390, 880}, ab = b - a;
        const float length = std::hypot(ab.x, ab.y);
        const sf::Vector2f n{-ab.y / length, ab.x / length};

        sf::VertexArray deck(sf::PrimitiveType::TriangleStrip, 4);
        deck[0].position = a - n * 50.f;
        deck[1].position = a + n * 50.f;
        deck[2].position = b - n * 50.f;
        deck[3].position = b + n * 50.f;
        for (std::size_t i = 0; i < 4; ++i) {
            deck[i].color = {30, 225, 190, 65};
        }
        sceneTarget->draw(deck);

        for (float side : {-61.f, 61.f}) {
            sf::VertexArray rail(sf::PrimitiveType::Lines, 2);
            rail[0].position = a + n * side;
            rail[1].position = b + n * side;
            rail[0].color = rail[1].color = {255, 75, 65, 235};
            sceneTarget->draw(rail);
        }

        rectangle({{158, 830}, {132, 87}}, {60, 230, 110, 40}, {80, 255, 145, 205});

        auto marker = [&](sf::Vector2f p, float radius, sf::Color color) {
            sf::CircleShape c(radius);
            c.setOrigin({radius, radius});
            c.setPosition(p);
            c.setFillColor(color);
            sceneTarget->draw(c);
        };
        marker(KingdomMap::SPAWN, 7, {255, 220, 35, 245});
        marker(position, 5, {255, 255, 255, 250});

        const auto foot = avatar ? avatar->getFootCollider() : KingdomFootModel::playerCollider(position);
        rectangle(foot, {255, 240, 45, 85}, {255, 245, 100, 245});

        for (const auto& n : npcs) {
            rectangle(n->getFootCollider(), {255, 145, 35, 80}, {255, 175, 65, 235});
        }

        sf::CircleShape interaction(KingdomMap::CAVE_INTERACTION_RADIUS);
        interaction.setOrigin({KingdomMap::CAVE_INTERACTION_RADIUS, KingdomMap::CAVE_INTERACTION_RADIUS});
        interaction.setPosition(KingdomMap::CAVE_CENTER);
        interaction.setFillColor({180, 70, 255, 18});
        interaction.setOutlineColor({210, 110, 255, 170});
        interaction.setOutlineThickness(2);
        sceneTarget->draw(interaction);

        if (renderer) {
            for (std::size_t i = 0; i < KingdomRenderer::OCCLUDER_COUNT; ++i) {
                const float y = renderer->occluderSortKey(i);
                sf::VertexArray baseline(sf::PrimitiveType::Lines, 2);
                baseline[0].position = {0, y};
                baseline[1].position = {KingdomMap::WORLD_SIZE.x, y};
                baseline[0].color = baseline[1].color = {215, 85, 255, 90};
                sceneTarget->draw(baseline);
            }
        }
    }

    compositor->finish();
    w.clear();
    w.setView(uiView);
    compositor->drawWorld(w);
    compositor->drawWeather(w, environment.getWeather(), elapsed);

    // Render off-screen objective indicator arrow
    if (navigationActive) {
        const sf::Vector2f camCenter = worldView.getCenter();
        const sf::Vector2f viewHalf = worldView.getSize() / 2.f;
        const sf::Vector2f dir = targetPos - position;
        const float dist = std::hypot(dir.x, dir.y);

        sf::FloatRect viewBounds{camCenter - viewHalf, worldView.getSize()};
        if (!viewBounds.contains(targetPos) && dist > 1.f) {
            const sf::Vector2f normDir = dir / dist;
            sf::Vector2f arrowPos{640.f + normDir.x * 320.f, 360.f + normDir.y * 220.f};
            arrowPos.x = std::clamp(arrowPos.x, 60.f, 1220.f);
            arrowPos.y = std::clamp(arrowPos.y, 100.f, 660.f);

            const float angleDeg = std::atan2(normDir.y, normDir.x) * 180.f / 3.14159265f;
            const float arrowPulse = 0.85f + 0.15f * std::sin(elapsed * 5.f);

            sf::ConvexShape arrow(3);
            arrow.setPoint(0, {15.f, 0.f});
            arrow.setPoint(1, {-10.f, -8.f});
            arrow.setPoint(2, {-10.f, 8.f});
            arrow.setPosition(arrowPos);
            arrow.setRotation(sf::degrees(angleDeg));
            arrow.setFillColor(sf::Color(255, 210, 60, static_cast<std::uint8_t>(235.f * arrowPulse)));
            arrow.setOutlineColor(sf::Color(10, 20, 30, 220));
            arrow.setOutlineThickness(1.5f);
            w.draw(arrow);

            drawText(w, std::to_string(static_cast<int>(dist)) + "m", arrowPos + sf::Vector2f(0.f, 18.f), 14, sf::Color(255, 225, 110, 230), true);
        }
    }

    // Render Asterfall Kingdom Entry Location Reveal
    if (entryRevealTimer < 2.5f) {
        float revealAlpha = 1.f;
        if (entryRevealTimer < 0.45f) {
            revealAlpha = entryRevealTimer / 0.45f;
        } else if (entryRevealTimer < 1.8f) {
            revealAlpha = 1.f;
        } else {
            revealAlpha = 1.f - (entryRevealTimer - 1.8f) / 0.7f;
        }
        revealAlpha = std::clamp(revealAlpha, 0.f, 1.f);
        const std::uint8_t a = static_cast<std::uint8_t>(255.f * revealAlpha);

        sf::RectangleShape darkOverlay({1280.f, 720.f});
        darkOverlay.setFillColor(sf::Color(4, 7, 14, static_cast<std::uint8_t>(240.f * revealAlpha)));
        w.draw(darkOverlay);

        drawText(w, "ASTERFALL KINGDOM", {640.f, 320.f}, 52, sf::Color(245, 212, 115, a), true);
        drawText(w, "The Last Bastion", {640.f, 382.f}, 22, sf::Color(185, 215, 225, a), true);

        sf::RectangleShape goldenLine({220.f, 2.f});
        goldenLine.setOrigin({110.f, 1.f});
        goldenLine.setPosition({640.f, 416.f});
        goldenLine.setFillColor(sf::Color(218, 175, 85, static_cast<std::uint8_t>(200.f * revealAlpha)));
        w.draw(goldenLine);
    }

    // Objective HUD (Top Left)
    sf::RectangleShape objective({370, 76});
    objective.setPosition({24, 24});
    objective.setFillColor({5, 14, 22, 205});
    objective.setOutlineColor({226, 184, 92, 190});
    objective.setOutlineThickness(1);
    w.draw(objective);

    drawText(w, "OBJECTIVE", {44, 34}, 15, {238, 201, 112});
    drawText(
        w,
        gate.getState() == GateState::Open ? "Reach the Ruined Catacombs" : "Open the Castle Gate",
        {44, 61},
        21,
        {235, 241, 234}
    );

    drawText(
        w,
        "WASD Move    E Interact    V Navigate    ESC Main Menu",
        {28, 672},
        14,
        {220, 224, 215, 205}
    );
    drawText(
        w,
        "F3 Navigation Debug    F4 Time    F5 Weather    F6 Performance",
        {28, 694},
        13,
        {180, 195, 200, 190}
    );

    if (navigationActive) {
        drawText(w, "NAVIGATION ACTIVE", {44, 108}, 13, sf::Color(255, 210, 60, 220));
    }

    if (debugCollision) {
        drawText(
            w,
            "F3 NAV  foot (" + std::to_string(int(position.x)) + ", " + std::to_string(int(position.y)) +
                ")  green=walkable  red=solid/rail  blue=water  cyan=deck  orange=NPC  purple=catacomb",
            {405, 30},
            14,
            {255, 245, 190}
        );
    }

    if (debugPerformance) {
        const char* phases[] = {"Dawn", "Day", "Sunset", "Night"};
        const char* weather[] = {"Clear", "Rain", "Mist"};
        drawText(
            w,
            "FPS " + std::to_string(int(lastFrameDt > 0 ? 1 / lastFrameDt : 0)) + "  frame " +
                std::to_string(lastFrameDt * 1000).substr(0, 5) + " ms\nphase " + phases[int(dayNight.phase())] +
                "  weather " + weather[int(environment.getWeather())] + "\ncache " +
                std::to_string(textures.getCacheHits()) + " hit / " + std::to_string(textures.getCacheMisses()) +
                " miss",
            {930, 28},
            14,
            {235, 240, 225}
        );
    }

    if (promptAlpha > 0) {
        sf::RectangleShape panel({330, 44});
        panel.setPosition({475, 646});
        panel.setFillColor({6, 15, 22, std::uint8_t(205 * promptAlpha)});
        w.draw(panel);
        drawText(
            w,
            gate.getState() == GateState::Closed ? "E   Open the Gate" : "E   Enter Ruined Catacombs",
            {492, 656},
            17,
            {240, 239, 226, std::uint8_t(255 * promptAlpha)}
        );
    }

    sf::RectangleShape veil({1280, 720});
    veil.setFillColor({3, 7, 12, std::uint8_t(255 * std::clamp(fade, 0.f, 1.f))});
    w.draw(veil);
}
