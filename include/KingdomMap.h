#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <string_view>
#include <vector>

enum class KingdomSurface { Grass,Road,Stone,Bridge,Stairs,ShallowDecoration,Water,Wall,Building,Cliff,Blocked };
struct KingdomCell { KingdomSurface surface=KingdomSurface::Grass;bool playerWalkable=true;bool npcWalkable=true;float heightLevel=0.f;bool walkable()const{return playerWalkable;} };
struct KingdomSolidFootprint { sf::FloatRect bounds; std::string_view name; };
struct KingdomWalkableRegion { sf::FloatRect bounds; std::string_view name; };

class KingdomMap {
public:
 static constexpr sf::Vector2f WORLD_SIZE{1672,941},SPAWN{224,884},RETURN_SPAWN{1350,355},CAVE_CENTER{1435,238},GATE_CENTER{224,850};
 static constexpr float INTERACTION_RADIUS=92, CAVE_INTERACTION_RADIUS=120.f, GATE_INTERACTION_RADIUS=88;
 static constexpr int CELL_SIZE=32,GRID_WIDTH=53,GRID_HEIGHT=30;
 static constexpr sf::FloatRect GATE_BLOCKER{{164,815},{112,34}};
 static constexpr std::array<sf::Vector2f,14> GOLDEN_ROUTE{{{224,884},{224,780},{430,680},{650,650},{900,600},{1020,520},{1060,355},{1210,355},{1250,350},{1350,355},{1350,375},{1435,375},{1435,300},{1435,238}}};
 KingdomMap();
 sf::Vector2f resolveMovement(sf::Vector2f,sf::Vector2f,float radius=18,bool gateOpen=true)const;
 sf::Vector2f resolveMovementWithActors(sf::Vector2f,sf::Vector2f,const std::vector<sf::Vector2f>&,float radius=14,float actorRadius=12,bool gateOpen=true)const;
 bool canInteract(sf::Vector2f)const;bool canInteractWithGate(sf::Vector2f)const;
 bool isBlocked(sf::Vector2f,float radius=18,bool gateOpen=true)const;
 bool canStandAt(sf::Vector2f,float radius=14,bool gateOpen=true)const;
 bool canStandAt(sf::FloatRect footprint,bool gateOpen=true)const;
 bool isOnBridge(sf::Vector2f,float radius=0)const;
 bool isNpcFootprintWalkable(sf::FloatRect)const;
 bool isWalkable(sf::Vector2f)const;bool isNpcWalkable(sf::Vector2f)const;bool isWater(sf::Vector2f)const;float heightAt(sf::Vector2f)const;
 const KingdomCell& cellAt(int,int)const;
 const std::vector<sf::FloatRect>& getObstacles()const{return obstacles;}
 const std::vector<KingdomSolidFootprint>& getSolidFootprints()const{return solidFootprints;}
 const std::vector<KingdomWalkableRegion>& getWalkableRegions()const{return walkableRegions;}
 const std::vector<sf::FloatRect>& getWaterZones()const{return waterZones;}
private:
 std::vector<KingdomSolidFootprint> solidFootprints;
 std::vector<KingdomWalkableRegion> walkableRegions;
 std::vector<sf::FloatRect> obstacles,waterZones;std::vector<KingdomCell> cells;
 bool blocked(sf::Vector2f,float,bool)const;
 bool blocked(sf::FloatRect,bool)const;
 bool inWalkableRegion(sf::Vector2f)const;
};
