#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <array>

enum class KingdomTileType { Ground,Road,Bridge,Stairs,Water,Wall,Building,Cliff,DecorationBlocked };
struct KingdomCell { KingdomTileType type=KingdomTileType::Ground;bool walkable=true;bool blocksNPC=false; };

class KingdomMap {
public:
    static constexpr sf::Vector2f WORLD_SIZE{1672.f, 941.f};
    static constexpr sf::Vector2f SPAWN{220.f, 890.f};
    static constexpr sf::Vector2f RETURN_SPAWN{1260.f, 330.f};
    static constexpr sf::Vector2f CAVE_CENTER{1395.f, 190.f};
    static constexpr float INTERACTION_RADIUS = 92.f;
    static constexpr sf::Vector2f GATE_CENTER{220.f, 850.f};
    static constexpr float GATE_INTERACTION_RADIUS = 88.f;
    static constexpr int CELL_SIZE=32;
    static constexpr int GRID_WIDTH=53,GRID_HEIGHT=30;
    static constexpr sf::FloatRect GATE_BLOCKER{{164.f, 815.f}, {112.f, 34.f}};

    KingdomMap();
    sf::Vector2f resolveMovement(sf::Vector2f position,
                                 sf::Vector2f displacement,
                                 float radius = 18.f,
                                 bool gateOpen = true) const;
    bool canInteract(sf::Vector2f position) const;
    bool canInteractWithGate(sf::Vector2f position) const;
    bool isBlocked(sf::Vector2f position, float radius = 18.f,
                   bool gateOpen = true) const;
    const std::vector<sf::FloatRect>& getObstacles() const { return obstacles; }
    const std::vector<sf::FloatRect>& getWaterZones() const { return waterZones; }
    bool isOnBridge(sf::Vector2f position, float radius = 0.f) const;
    const KingdomCell& cellAt(int x,int y)const;
    bool isWalkable(sf::Vector2f position)const;
    static constexpr std::array<sf::Vector2f,7> GOLDEN_ROUTE{{
        {430,740},{560,665},{760,610},{1010,660},{1160,750},{1320,845},{1395,190}}};
private:
    std::vector<sf::FloatRect> obstacles;
    std::vector<sf::FloatRect> waterZones;
    std::vector<KingdomCell> cells;
    bool blocked(sf::Vector2f position, float radius, bool gateOpen) const;
};
