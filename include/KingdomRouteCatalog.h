#pragma once
#include "DayNightSystem.h"
#include "KingdomNpc.h"
#include <vector>
struct KingdomNpcRouteDefinition{KingdomNpcRole role;std::vector<sf::Vector2f>route;std::vector<sf::Vector2f>phaseDestinations;};
inline const std::vector<KingdomNpcRouteDefinition>&kingdomNpcRoutes(){
 static const std::vector<KingdomNpcRouteDefinition> routes={
  {KingdomNpcRole::Guard,{{180,880},{220,880}},{{180,880},{220,880},{180,880},{220,880}}},
  {KingdomNpcRole::Guard,{{440,850},{470,810}},{{440,850},{470,810},{440,850},{470,810}}},
  {KingdomNpcRole::Guard,{{430,720},{610,650},{650,620}},{{430,720},{610,650},{650,620},{430,720}}},
  {KingdomNpcRole::Guard,{{1205,355},{1250,355}},{{1205,355},{1250,355},{1205,355},{1250,355}}},
  {KingdomNpcRole::Citizen,{{520,470},{700,470},{650,570}},{{520,470},{700,470},{650,570},{520,470}}},
  {KingdomNpcRole::Citizen,{{420,390},{560,350}},{{420,390},{560,350},{420,390},{420,390}}},
  {KingdomNpcRole::Citizen,{{760,310},{850,350},{820,450}},{{760,310},{850,350},{820,450},{760,310}}},
  {KingdomNpcRole::Citizen,{{560,180},{610,190}},{{560,180},{610,190},{560,180},{560,180}}}
 };return routes;
}
inline sf::Vector2f kingdomPhaseDestination(const KingdomNpcRouteDefinition&r,DayPhase p){return r.phaseDestinations[static_cast<std::size_t>(p)];}
