#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <string_view>
#include <vector>

enum class KingdomSurface { Grass,Road,Stone,Bridge,Stairs,ShallowDecoration,Water,Wall,Building,Cliff,Blocked };
struct KingdomCell { KingdomSurface surface=KingdomSurface::Grass;bool playerWalkable=true;bool npcWalkable=true;float heightLevel=0.f;bool walkable()const{return playerWalkable;} };
struct KingdomSolidFootprint { sf::FloatRect bounds; std::string_view name; };

class KingdomMap {
public:
 static constexpr sf::Vector2f WORLD_SIZE{1672,941},SPAWN{224,884},RETURN_SPAWN{1360,350},CAVE_CENTER{1435,238},GATE_CENTER{224,850};
 static constexpr float INTERACTION_RADIUS=92,GATE_INTERACTION_RADIUS=88;
 static constexpr int CELL_SIZE=32,GRID_WIDTH=53,GRID_HEIGHT=30;
 static constexpr sf::FloatRect GATE_BLOCKER{{164,815},{112,34}};
 static constexpr std::array<sf::Vector2f,7> GOLDEN_ROUTE{{{430,740},{560,665},{760,610},{1010,660},{1160,750},{1320,845},{1395,190}}};
 KingdomMap();
 sf::Vector2f resolveMovement(sf::Vector2f,sf::Vector2f,float radius=18,bool gateOpen=true)const;
 bool canInteract(sf::Vector2f)const;bool canInteractWithGate(sf::Vector2f)const;
 bool isBlocked(sf::Vector2f,float radius=18,bool gateOpen=true)const;
 bool isOnBridge(sf::Vector2f,float radius=0)const;
 bool isNpcFootprintWalkable(sf::FloatRect)const;
 bool isWalkable(sf::Vector2f)const;bool isNpcWalkable(sf::Vector2f)const;bool isWater(sf::Vector2f)const;float heightAt(sf::Vector2f)const;
 const KingdomCell& cellAt(int,int)const;
 const std::vector<sf::FloatRect>& getObstacles()const{return obstacles;}
 const std::vector<KingdomSolidFootprint>& getSolidFootprints()const{return solidFootprints;}
 const std::vector<sf::FloatRect>& getWaterZones()const{return waterZones;}
private:
 std::vector<KingdomSolidFootprint> solidFootprints;
 std::vector<sf::FloatRect> obstacles,waterZones;std::vector<KingdomCell> cells;
 bool blocked(sf::Vector2f,float,bool)const;
};
