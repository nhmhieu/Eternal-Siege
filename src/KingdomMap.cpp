#include "KingdomMap.h"
#include <algorithm>
#include <cmath>

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

    // All authored values are artwork pixels in the same 1672x941 world space.
    // Only ground-contact footprints are solid; roofs and tree crowns are depth layers.
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

    cells.resize(GRID_WIDTH * GRID_HEIGHT);
    for (int y = 0; y < GRID_HEIGHT; ++y) {
        for (int x = 0; x < GRID_WIDTH; ++x) {
            const sf::Vector2f p{(x + .5f) * CELL_SIZE, (y + .5f) * CELL_SIZE};
            auto& c = cells[y * GRID_WIDTH + x];
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
    const sf::Vector2f a{1030, 655}, b{1390, 880}, ab = b - a, ap = p - a;
    const float t = std::clamp((ap.x * ab.x + ap.y * ab.y) / (ab.x * ab.x + ab.y * ab.y), 0.f, 1.f);
    const auto d = p - (a + ab * t);
    return d.x * d.x + d.y * d.y <= std::pow(std::max(0.f, 58 - r), 2);
}

bool KingdomMap::inWalkableRegion(sf::Vector2f p) const {
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
    return blocked(sf::FloatRect{{p.x - r, p.y - r}, {2 * r, 2 * r}}, gateOpen);
}

bool KingdomMap::blocked(sf::FloatRect footprint, bool gateOpen) const {
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
    const int steps = std::max(1, int(std::ceil(std::hypot(d.x, d.y) / 10)));
    const auto step = d / float(steps);

    for (int i = 0; i < steps; ++i) {
        const sf::Vector2f x{p.x + step.x, p.y};
        if (!blocked(x, r, gateOpen)) {
            p.x = x.x;
        }
        const sf::Vector2f y{p.x, p.y + step.y};
        if (!blocked(y, r, gateOpen)) {
            p.y = y.y;
        }
    }
    return p;
}

sf::Vector2f KingdomMap::resolveMovementWithActors(
    sf::Vector2f p, sf::Vector2f d, const std::vector<sf::Vector2f>& actors,
    float r, float actorRadius, bool gateOpen) const
{
    const int steps = std::max(1, int(std::ceil(std::hypot(d.x, d.y) / 10)));
    const auto step = d / float(steps);

    auto clear = [&](sf::Vector2f candidate) {
        if (blocked(candidate, r, gateOpen)) {
            return false;
        }
        for (const auto& a : actors) {
            if (std::hypot(candidate.x - a.x, candidate.y - a.y) < r + actorRadius) {
                return false;
            }
        }
        return true;
    };

    for (int i = 0; i < steps; ++i) {
        const sf::Vector2f x{p.x + step.x, p.y};
        if (clear(x)) {
            p.x = x.x;
        }
        const sf::Vector2f y{p.x, p.y + step.y};
        if (clear(y)) {
            p.y = y.y;
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
