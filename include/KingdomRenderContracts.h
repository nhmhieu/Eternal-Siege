#pragma once
#include "KingdomMap.h"
#include <array>
#include <cmath>
enum class KingdomRenderPass{WorldBase,Water,Entities,Foreground,Lighting,Weather,Ui};
struct KingdomRenderKey{float footY;float height;int category;};
inline float kingdomSortKey(float footY,float){return footY;}
inline bool kingdomRenderBefore(const KingdomRenderKey&a,const KingdomRenderKey&b){const float ak=kingdomSortKey(a.footY,a.height),bk=kingdomSortKey(b.footY,b.height);return ak==bk?a.category<b.category:ak<bk;}
inline bool kingdomUiAffectedBy(KingdomRenderPass pass){return pass==KingdomRenderPass::Ui;}
inline bool kingdomRainRippleAllowed(const KingdomMap&map,sf::Vector2f p){return map.isWater(p);}
inline bool kingdomActuallyMoving(sf::Vector2f d){return std::hypot(d.x,d.y)>.001f;}
inline constexpr std::array<float,10>KINGDOM_OCCLUDER_KEYS{{882,886,830,414,725,825,320,285,650,690}};
