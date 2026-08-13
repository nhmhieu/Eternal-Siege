#include "KingdomMap.h"
#include "AssetLocator.h"
#include <algorithm>
#include <cmath>
#include <iostream>

KingdomMap::KingdomMap() {
    walkableRegions = {
        {{{154, 820}, {140, 97}}, "gate opening"},
        {{{145, 700}, {385, 217}}, "gate courtyard"},
        {{{300, 515}, {760, 380}}, "lower plaza and road"},
        {{{390, 300}, {710, 360}}, "central square"},
        {{{500, 118}, {430, 260}}, "north village road"},
        {{{850, 250}, {455, 250}}, "east stone road"},
        {{{1080, 250}, {415, 165}}, "catacomb approach"},
        {{{1368, 96}, {134, 258}}, "catacomb stairs"},
        {{{920, 595}, {175, 155}}, "west bridge approach"},
        {{{1360, 790}, {180, 127}}, "east bridge approach"}
    };

    solidFootprints = {
        {{{0, 0}, {26, 941}}, "west world edge"}, {{{1646, 0}, {26, 941}}, "east world edge"},
        {{{0, 0}, {1672, 24}}, "north world edge"}, {{{0, 917}, {1672, 24}}, "south world edge"},
        {{{34, 793}, {118, 124}}, "left gate pillar"}, {{{298, 783}, {112, 134}}, "right gate pillar"},
        {{{0, 884}, {154, 33}}, "left gate wall"}, {{{404, 891}, {238, 26}}, "right gate wall"},
        {{{37, 176}, {122, 48}}, "northwest cottage foundation"}, {{{210, 166}, {143, 47}}, "north cottage foundation"},
        {{{386, 175}, {154, 45}}, "northeast cottage foundation"},
        {{{18, 444}, {170, 65}}, "tavern foundation"}, {{{197, 408}, {244, 75}}, "guildhall foundation"},
        {{{661, 376}, {137, 67}}, "fountain basin"},
        {{{395, 485}, {220, 95}}, "south flower bed"}, {{{875, 370}, {130, 115}}, "east flower bed"},
        {{{750, 700}, {205, 145}}, "rocks below bridge approach"},
        {{{471, 255}, {31, 34}}, "village tree trunk"}, {{{1128, 589}, {34, 39}}, "bridge tree trunk"},
        {{{1480, 626}, {38, 43}}, "river tree trunk"}, {{{846, 279}, {25, 31}}, "square tree trunk"},
        {{{965, 302}, {25, 30}}, "east square tree trunk"},
        {{{1095, 245}, {95, 95}}, "catacomb tree and rock cluster"},
        {{{1218, 24}, {164, 225}}, "catacomb west wall"}, {{{1512, 24}, {134, 310}}, "catacomb east wall"},
        {{{1218, 24}, {428, 73}}, "catacomb upper wall"}, {{{1228, 286}, {137, 46}}, "catacomb west buttress"},
        {{{1510, 302}, {136, 49}}, "catacomb east buttress"}, {{{1372, 316}, {33, 28}}, "catacomb west stair edge"},
        {{{1470, 316}, {37, 28}}, "catacomb east stair edge"},
        {{{1090, 371}, {55, 42}}, "river cliff west"}, {{{1568, 387}, {78, 48}}, "river cliff east"}
    };

    obstacles.reserve(solidFootprints.size());
    for (const auto& f : solidFootprints) {
        obstacles.push_back(f.bounds);
    }

    waterZones = {
        {{865, 785}, {335, 132}},
        {{1090, 690}, {330, 227}},
        {{1180, 410}, {255, 350}},
        {{1390, 410}, {256, 507}}
    };

    const auto maskPath = AssetLocator::find("assets/images/kingdom/kingdom_collision_mask.png");
    if (maskPath && collisionMaskImage.loadFromFile(*maskPath)) {
        if (collisionMaskImage.getSize().x == 1672 && collisionMaskImage.getSize().y == 941) {
            maskLoaded = true;
            std::cout << "[KingdomMap] Loaded PNG collision mask ("
                      << collisionMaskImage.getSize().x << "x" << collisionMaskImage.getSize().y << ")\n";
        } else {
            std::cerr << "[KingdomMap] Collision mask dimension mismatch! Expected 1672x941, got "
                      << collisionMaskImage.getSize().x << "x" << collisionMaskImage.getSize().y
                      << ". Using rectangle fallback.\n";
            maskLoaded = false;
        }
    } else {
        std::cerr << "[KingdomMap] Failed to load assets/images/kingdom/kingdom_collision_mask.png! Using rectangle fallback.\n";
        maskLoaded = false;
    }

    cells.resize(GRID_WIDTH * GRID_HEIGHT);
    for (int y = 0; y < GRID_HEIGHT; ++y) {
        for (int x = 0; x < GRID_WIDTH; ++x) {
            const sf::Vector2f p{(x + .5f) * CELL_SIZE, (y + .5f) * CELL_SIZE};
            auto& c = cells[y * GRID_WIDTH + x];
            if (maskLoaded) {
                const MaskSurface s = surfaceAtPixel(int(p.x), int(p.y));
                if (s == MaskSurface::Bridge) {
                    c.surface = KingdomSurface::Bridge;
                    c.heightLevel = 1;
                    c.playerWalkable = c.npcWalkable = true;
                } else if (s == MaskSurface::Water) {
                    c.surface = KingdomSurface::Water;
                    c.playerWalkable = c.npcWalkable = false;
                } else if (s == MaskSurface::Solid) {
                    c.surface = KingdomSurface::Blocked;
                    c.playerWalkable = c.npcWalkable = false;
                } else {
                    c.surface = KingdomSurface::Road;
                    c.playerWalkable = c.npcWalkable = true;
                }
            } else {
                if (isOnBridge(p)) {
                    c.surface = KingdomSurface::Bridge;
                    c.heightLevel = 1;
                    c.playerWalkable = c.npcWalkable = true;
                } else if (blocked(p, 8, true)) {
                    c.playerWalkable = c.npcWalkable = false;
                    c.surface = isWater(p) ? KingdomSurface::Water : KingdomSurface::Blocked;
                } else {
                    c.surface = KingdomSurface::Road;
                }
            }
        }
    }
}

KingdomMap::MaskSurface KingdomMap::surfaceAtPixel(int x, int y) const {
    if (x < 0 || y < 0 || x >= 1672 || y >= 941) {
        return MaskSurface::Solid;
    }
    if (!maskLoaded) {
        return MaskSurface::Walkable;
    }
    const sf::Color c = collisionMaskImage.getPixel(sf::Vector2u(x, y));
    if (c.r == 0 && c.g == 255 && c.b == 0) {
        return MaskSurface::Bridge;
    }
    if (c.r == 0 && c.g == 0 && c.b == 255) {
        return MaskSurface::Water;
    }
    if (c.r == 255 && c.g == 255 && c.b == 255) {
        return MaskSurface::Walkable;
    }
    return MaskSurface::Solid;
}

const KingdomCell& KingdomMap::cellAt(int x, int y) const {
    static const KingdomCell out{KingdomSurface::Cliff, false, false, 0};
    if (x < 0 || y < 0 || x >= GRID_WIDTH || y >= GRID_HEIGHT) {
        return out;
    }
    return cells[y * GRID_WIDTH + x];
}

bool KingdomMap::isWalkable(sf::Vector2f p) const {
    return canStandAt(p, 8, true);
}

bool KingdomMap::isNpcWalkable(sf::Vector2f p) const {
    return canStandAt(p, 8, true);
}

bool KingdomMap::isWater(sf::Vector2f p) const {
    if (maskLoaded) {
        return surfaceAtPixel(int(p.x), int(p.y)) == MaskSurface::Water;
    }
    if (isOnBridge(p)) {
        return false;
    }
    for (const auto& w : waterZones) {
        if (w.contains(p)) {
            return true;
        }
    }
    return false;
}

float KingdomMap::heightAt(sf::Vector2f p) const {
    if (p.x < 0 || p.y < 0) {
        return 0;
    }
    return cellAt(int(p.x) / CELL_SIZE, int(p.y) / CELL_SIZE).heightLevel;
}

bool KingdomMap::isOnBridge(sf::Vector2f p, float r) const {
    if (maskLoaded) {
        return surfaceAtPixel(int(p.x), int(p.y)) == MaskSurface::Bridge;
    }
    const sf::Vector2f a{1030, 655}, b{1390, 880}, ab = b - a, ap = p - a;
    const float t = std::clamp((ap.x * ab.x + ap.y * ab.y) / (ab.x * ab.x + ab.y * ab.y), 0.f, 1.f);
    const auto d = p - (a + ab * t);
    return d.x * d.x + d.y * d.y <= std::pow(std::max(0.f, 58 - r), 2);
}

bool KingdomMap::inWalkableRegion(sf::Vector2f p) const {
    if (maskLoaded) {
        const MaskSurface s = surfaceAtPixel(int(p.x), int(p.y));
        return s == MaskSurface::Walkable || s == MaskSurface::Bridge;
    }
    if (isOnBridge(p)) {
        return true;
    }
    for (const auto& r : walkableRegions) {
        if (r.bounds.contains(p)) {
            return true;
        }
    }
    return false;
}

bool KingdomMap::blocked(sf::Vector2f p, float r, bool gateOpen) const {
    if (maskLoaded) {
        const float effR = std::min(r, 11.5f);
        const float diag = effR * 0.70710678f;

        const sf::Vector2f probes[9] = {
            {p.x, p.y},
            {p.x + effR, p.y},
            {p.x - effR, p.y},
            {p.x, p.y + effR},
            {p.x, p.y - effR},
            {p.x + diag, p.y + diag},
            {p.x + diag, p.y - diag},
            {p.x - diag, p.y + diag},
            {p.x - diag, p.y - diag}
        };

        for (const auto& probe : probes) {
            const int px = static_cast<int>(std::floor(probe.x));
            const int py = static_cast<int>(std::floor(probe.y));
            const MaskSurface s = surfaceAtPixel(px, py);
            if (s == MaskSurface::Solid || s == MaskSurface::Water) {
                return true;
            }
        }

        const sf::FloatRect footBox{{p.x - effR, p.y - effR}, {2.f * effR, 2.f * effR}};
        if (!gateOpen && footBox.findIntersection(GATE_BLOCKER).has_value()) {
            return true;
        }

        return false;
    }

    return blocked(sf::FloatRect{{p.x - r, p.y - r}, {2 * r, 2 * r}}, gateOpen);
}

bool KingdomMap::blocked(sf::FloatRect footprint, bool gateOpen) const {
    if (maskLoaded) {
        const sf::Vector2f center = footprint.position + footprint.size * 0.5f;
        const float r = std::min(footprint.size.x, footprint.size.y) * 0.5f;
        return blocked(center, r, gateOpen);
    }

    // --- FALLBACK RECTANGLE COLLISION SYSTEM ---
    const auto samplePoint = [&](std::size_t x, std::size_t y) {
        return sf::Vector2f{
            footprint.position.x + footprint.size.x * (float(x) / 2.f),
            footprint.position.y + footprint.size.y * (float(y) / 2.f)
        };
    };

    for (std::size_t y = 0; y < 3; ++y) {
        for (std::size_t x = 0; x < 3; ++x) {
            const sf::Vector2f probe = samplePoint(x, y);
            if (!inWalkableRegion(probe) && !isOnBridge(probe)) {
                return true;
            }
        }
    }

    const sf::Vector2f center = footprint.position + footprint.size * 0.5f;
    for (const auto& o : obstacles) {
        if (footprint.findIntersection(o).has_value()) {
            return true;
        }
    }

    const sf::Vector2f a{1030, 655}, b{1390, 880}, ab = b - a;
    const float length = std::hypot(ab.x, ab.y);
    const sf::Vector2f normal{-ab.y / length, ab.x / length};

    auto nearRail = [&](float side, const sf::Vector2f& p) {
        const auto ra = a + normal * side, rb = b + normal * side, rab = rb - ra, ap = p - ra;
        const float t = std::clamp((ap.x * rab.x + ap.y * rab.y) / (rab.x * rab.x + rab.y * rab.y), 0.f, 1.f);
        const auto closest = ra + rab * t;
        return std::hypot((p - closest).x, (p - closest).y) < 10.f;
    };

    for (std::size_t y = 0; y < 3; ++y) {
        for (std::size_t x = 0; x < 3; ++x) {
            const sf::Vector2f p = samplePoint(x, y);
            if (nearRail(-61.f, p) || nearRail(61.f, p)) {
                return true;
            }
        }
    }

    if (!isOnBridge(center, 0)) {
        for (const auto& w : waterZones) {
            if (footprint.findIntersection(w).has_value()) {
                return true;
            }
        }
    }

    if (!gateOpen && footprint.findIntersection(GATE_BLOCKER).has_value()) {
        return true;
    }

    return false;
}

sf::Vector2f KingdomMap::resolveMovement(sf::Vector2f p, sf::Vector2f d, float r, bool gateOpen) const {
    std::vector<sf::Vector2f> emptyActors;
    return resolveMovementWithActors(p, d, emptyActors, r, 0.f, gateOpen);
}

sf::Vector2f KingdomMap::resolveMovementWithActors(
    sf::Vector2f p, sf::Vector2f d, const std::vector<sf::Vector2f>& actors,
    float r, float actorRadius, bool gateOpen) const
{
    const float effR = std::min(r, 11.5f);
    const int steps = std::max(1, int(std::ceil(std::hypot(d.x, d.y) / 4.f)));
    const auto step = d / float(steps);

    auto clear = [&](sf::Vector2f candidate) {
        if (blocked(candidate, effR, gateOpen)) {
            return false;
        }
        for (const auto& a : actors) {
            if (std::hypot(candidate.x - a.x, candidate.y - a.y) < effR + actorRadius) {
                return false;
            }
        }
        return true;
    };

    for (int i = 0; i < steps; ++i) {
        const sf::Vector2f fullCandidate{p.x + step.x, p.y + step.y};
        if (clear(fullCandidate)) {
            p = fullCandidate;
            continue;
        }

        const sf::Vector2f candX{p.x + step.x, p.y};
        const bool clearX = (step.x != 0.f) && clear(candX);

        const sf::Vector2f candY{p.x, p.y + step.y};
        const bool clearY = (step.y != 0.f) && clear(candY);

        if (clearX && !clearY) {
            p = candX;
        } else if (clearY && !clearX) {
            p = candY;
        } else if (clearX && clearY) {
            if (std::abs(step.x) >= std::abs(step.y)) {
                p = candX;
            } else {
                p = candY;
            }
        }
    }
    return p;
}

bool KingdomMap::isBlocked(sf::Vector2f p, float r, bool gateOpen) const {
    return blocked(p, r, gateOpen);
}

bool KingdomMap::canStandAt(sf::Vector2f p, float r, bool gateOpen) const {
    return p.x >= 0 && p.y >= 0 && p.x < WORLD_SIZE.x && p.y < WORLD_SIZE.y && !blocked(p, r, gateOpen);
}

bool KingdomMap::canStandAt(sf::FloatRect footprint, bool gateOpen) const {
    const sf::Vector2f end = footprint.position + footprint.size;
    return footprint.position.x >= 0 && footprint.position.y >= 0 &&
           end.x <= WORLD_SIZE.x && end.y <= WORLD_SIZE.y &&
           !blocked(footprint, gateOpen);
}

bool KingdomMap::canInteract(sf::Vector2f p) const {
    return canStandAt(p, 8, true) && std::hypot(p.x - CAVE_CENTER.x, p.y - CAVE_CENTER.y) <= CAVE_INTERACTION_RADIUS;
}

bool KingdomMap::canInteractWithGate(sf::Vector2f p) const {
    return std::hypot(p.x - GATE_CENTER.x, p.y - GATE_CENTER.y) <= GATE_INTERACTION_RADIUS;
}

bool KingdomMap::isNpcFootprintWalkable(sf::FloatRect b) const {
    const auto c = b.position + b.size / 2.f;
    const float radius = std::max(b.size.x, b.size.y) * .5f;
    return canStandAt(c, radius, true);
}
