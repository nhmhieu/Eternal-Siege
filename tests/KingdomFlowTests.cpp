#include "KingdomMap.h"
#include "LevelDefinition.h"
#include "GameProgress.h"
#include "KingdomGateController.h"
#include "DayNightSystem.h"
#include <cassert>
#include <cmath>

int main(){
 KingdomMap map;
 DayNightSystem cycle;const auto initial=cycle.phase();cycle.advancePhase();assert(cycle.phase()!=initial);cycle.update(DayNightSystem::CYCLE_SECONDS);assert(cycle.phase()!=initial);
 const auto start=KingdomMap::SPAWN;
 assert(!map.isBlocked(start,18.f,false));
 assert(map.isWalkable(start));
 assert(map.isBlocked({1300.f,520.f},18.f,true));
 const sf::Vector2f bridgeMid{1210.f,768.f};
 assert(map.isOnBridge(bridgeMid,18.f));
 assert(!map.isBlocked(bridgeMid,18.f,true));
 for(const auto waypoint:KingdomMap::GOLDEN_ROUTE)
  assert(map.isWalkable(waypoint));
 KingdomGateController gate;
 assert(gate.getState()==GateState::Closed&&gate.blocksPassage());
 assert(!gate.tryOpen(false));
 assert(gate.tryOpen(true));
 assert(!gate.tryOpen(true));
 gate.update(.4f);assert(gate.blocksPassage());
 gate.update(.6f);assert(!gate.blocksPassage()&&gate.getState()==GateState::Open);
 const sf::Vector2f diagonal{230.f/std::sqrt(2.f),230.f/std::sqrt(2.f)};
 const auto d=map.resolveMovement(start,diagonal);
 const auto delta=d-start;assert(std::sqrt(delta.x*delta.x+delta.y*delta.y)<=230.01f);
 const auto bounded=map.resolveMovement(start,{-5000.f,0.f});assert(bounded.x>=44.f);
 assert(!map.canInteract(KingdomMap::SPAWN));
 assert(map.canInteract(KingdomMap::CAVE_CENTER));
 assert(!map.canInteract(KingdomMap::RETURN_SPAWN));
 assert(LEVEL_DEFINITIONS[0].unlocked);
 assert(LEVEL_DEFINITIONS[0].id==LevelId::RuinedCatacombs);
 assert(LEVEL_DEFINITIONS[0].clearReward==300);
 assert(!LEVEL_DEFINITIONS[1].unlocked&&!LEVEL_DEFINITIONS[2].unlocked);
 GameProgress progress;const auto result=progress.grantVictory(LevelId::RuinedCatacombs,300);
 assert(progress.totalGold==300&&progress.ruinedCatacombsCleared);
 assert(result.goldEarned==300&&result.totalGold==300&&result.firstClear);
 return 0;
}
