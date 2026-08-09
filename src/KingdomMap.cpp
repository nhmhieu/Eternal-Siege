#include "KingdomMap.h"
#include <algorithm>
#include <cmath>

KingdomMap::KingdomMap() {
    obstacles = {
        {{0.f, 0.f}, {26.f, 941.f}}, {{1646.f, 0.f}, {26.f, 941.f}},
        {{0.f, 0.f}, {1672.f, 24.f}}, {{0.f, 917.f}, {1672.f, 24.f}},
        {{0.f, 45.f}, {525.f, 285.f}},
        {{0.f, 270.f}, {390.f, 270.f}},
        {{0.f, 650.f}, {145.f, 267.f}},
        {{300.f, 650.f}, {110.f, 220.f}},
        {{1230.f, 355.f}, {410.f, 190.f}},
        {{1215.f, 24.f}, {431.f, 95.f}},
        {{1485.f, 105.f}, {160.f, 250.f}}
    };
    waterZones = {
        {{865.f, 785.f}, {335.f, 132.f}},
        {{1090.f, 690.f}, {330.f, 227.f}},
        {{1180.f, 410.f}, {255.f, 350.f}},
        {{1390.f, 360.f}, {256.f, 557.f}}
    };
    cells.resize(GRID_WIDTH*GRID_HEIGHT);for(int y=0;y<GRID_HEIGHT;++y)for(int x=0;x<GRID_WIDTH;++x){const sf::Vector2f p{(x+.5f)*CELL_SIZE,(y+.5f)*CELL_SIZE};auto&c=cells[y*GRID_WIDTH+x];if(isOnBridge(p)){c.type=KingdomTileType::Bridge;c.walkable=true;}else if(blocked(p,8.f,true)){c.walkable=false;c.blocksNPC=true;c.type=KingdomTileType::DecorationBlocked;for(const auto&w:waterZones)if(w.contains(p))c.type=KingdomTileType::Water;}else c.type=KingdomTileType::Road;}
}

const KingdomCell& KingdomMap::cellAt(int x,int y)const{static const KingdomCell outside{KingdomTileType::Cliff,false,true};if(x<0||y<0||x>=GRID_WIDTH||y>=GRID_HEIGHT)return outside;return cells[y*GRID_WIDTH+x];}
bool KingdomMap::isWalkable(sf::Vector2f p)const{return cellAt(static_cast<int>(p.x)/CELL_SIZE,static_cast<int>(p.y)/CELL_SIZE).walkable&&!blocked(p,8.f,true);}

bool KingdomMap::isOnBridge(sf::Vector2f p,float radius)const{
    const sf::Vector2f a{1030.f,655.f},b{1390.f,880.f},ab=b-a,ap=p-a;
    const float t=std::clamp((ap.x*ab.x+ap.y*ab.y)/(ab.x*ab.x+ab.y*ab.y),0.f,1.f);
    const sf::Vector2f nearest=a+ab*t,d=p-nearest;
    return d.x*d.x+d.y*d.y<=std::pow(std::max(0.f,58.f-radius),2.f);
}

bool KingdomMap::blocked(sf::Vector2f p, float r, bool gateOpen) const {
    const sf::FloatRect avatar{{p.x-r, p.y-r}, {r*2.f, r*2.f}};
    for (const auto& wall : obstacles)
        if (wall.findIntersection(avatar)) return true;
    if(!isOnBridge(p,r))for(const auto& water:waterZones)if(water.findIntersection(avatar))return true;
    if (!gateOpen && GATE_BLOCKER.findIntersection(avatar)) return true;
    return false;
}

sf::Vector2f KingdomMap::resolveMovement(sf::Vector2f p,
                                         sf::Vector2f displacement,
                                         float radius, bool gateOpen) const {
    const float distance = std::sqrt(displacement.x*displacement.x +
                                     displacement.y*displacement.y);
    const int steps = std::max(1, static_cast<int>(std::ceil(distance / 10.f)));
    const sf::Vector2f step = displacement / static_cast<float>(steps);
    for (int i=0; i<steps; ++i) {
        sf::Vector2f x{p.x + step.x, p.y};
        if (!blocked(x, radius, gateOpen)) p.x = x.x;
        sf::Vector2f y{p.x, p.y + step.y};
        if (!blocked(y, radius, gateOpen)) p.y = y.y;
    }
    return p;
}

bool KingdomMap::canInteractWithGate(sf::Vector2f p) const {
    const sf::Vector2f d = p - GATE_CENTER;
    return d.x*d.x + d.y*d.y <= GATE_INTERACTION_RADIUS*GATE_INTERACTION_RADIUS;
}

bool KingdomMap::isBlocked(sf::Vector2f p, float r, bool gateOpen) const {
    return blocked(p, r, gateOpen);
}

bool KingdomMap::canInteract(sf::Vector2f p) const {
    const sf::Vector2f d = p - CAVE_CENTER;
    return d.x*d.x + d.y*d.y <= INTERACTION_RADIUS*INTERACTION_RADIUS;
}
