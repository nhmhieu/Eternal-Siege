#include "KingdomCompositor.h"
#include "KingdomFootModel.h"
#include "KingdomMap.h"
#include "KingdomNpc.h"
#include "KingdomRenderContracts.h"
#include "KingdomRouteCatalog.h"
#include "KingdomTransitionLatch.h"
#include "NpcScheduleSystem.h"
#include <cassert>
#include <cmath>
#include <filesystem>
#include <iostream>

static bool sameRect(sf::FloatRect a,sf::FloatRect b){return a.position==b.position&&a.size==b.size;}
static void requireNpcSafe(const KingdomMap&m,const KingdomNpc&n){assert(m.isNpcWalkable(n.getFootPosition()));assert(!m.isWater(n.getFootPosition()));if(!m.isNpcFootprintWalkable(n.getFootCollider()))std::cerr<<"unsafe NPC footprint at "<<n.getFootPosition().x<<","<<n.getFootPosition().y<<"\n";assert(m.isNpcFootprintWalkable(n.getFootCollider()));}
static sf::Image frame(KingdomCompositor&c,sf::Color ambient,KingdomWeather weather){c.ensure({64,64});sf::View v(sf::FloatRect{{0,0},{64,64}});auto&w=c.begin(v,{30,120,60});sf::RectangleShape p({64,64});p.setFillColor({30,120,60});w.draw(p);c.applyAmbient(ambient);c.finish();sf::RenderTexture out({64,64});out.clear();c.drawWorld(out);c.drawWeather(out,weather,.37f);sf::RectangleShape ui({14,14});ui.setPosition({48,48});ui.setFillColor({240,35,80});out.draw(ui);out.display();return out.getTexture().copyToImage();}
int main(){
 KingdomMap map;KingdomTransitionLatch latch;assert(latch.request());for(int i=0;i<10;++i)assert(!latch.request());
 struct LandmarkProbe{const char*name;sf::Vector2f pixel;bool walkable;};
 // These coordinates were read from kingdom_world_open.png, not obtained from
 // KingdomMap or its footprint catalog. They are visual truth probes.
 const LandmarkProbe landmarks[]={
  {"gate entrance road centre",{224,884},true},{"left gate pillar foot",{92,850},false},
  {"right gate pillar foot",{352,850},false},{"village tree trunk",{486,272},false},
  {"outside village crown",{525,254},true},{"bridge tree trunk",{1145,610},false},
  {"river tree trunk",{1498,648},false},{"fountain stone basin",{730,410},false},
  {"guildhall foundation",{310,450},false},{"catacomb west wall",{1280,200},false},
  {"catacomb open stair",{1435,238},true},{"bridge deck centre",{1210,768},true},
  {"bridge northwest rail",{1242,716},false},{"bridge southeast rail",{1178,820},false},
  {"river beside bridge",{1300,600},false}
 };
 for(const auto&p:landmarks){if(map.isWalkable(p.pixel)!=p.walkable)std::cerr<<"landmark mismatch: "<<p.name<<" at "<<p.pixel.x<<","<<p.pixel.y<<"\n";assert(map.isWalkable(p.pixel)==p.walkable);}
 assert(map.isWalkable(KingdomMap::SPAWN));assert(map.isWalkable(KingdomMap::RETURN_SPAWN));for(auto p:KingdomMap::GOLDEN_ROUTE)assert(map.isWalkable(p));
 assert(map.isBlocked({1300,520},18,true));for(int i=0;i<=10;++i){auto p=sf::Vector2f{1030,655}+(sf::Vector2f{1390,880}-sf::Vector2f{1030,655})*(i/10.f);assert(map.isOnBridge(p,18));assert(!map.isBlocked(p,18,true));}assert(map.isBlocked({1210,700},18,true));assert(map.isBlocked({1160,825},18,true));
 const auto tunnel=map.resolveMovement({900,700},{700,0},18,true);assert(tunnel.x<1180);const sf::Vector2f start{420,600};const auto slide=map.resolveMovement(start,{-120,100},18,true);assert(slide.y>start.y||slide.x<start.x);const sf::Vector2f diagonal{230/std::sqrt(2.f),230/std::sqrt(2.f)};assert(std::hypot(diagonal.x,diagonal.y)<=230.001f);assert(!kingdomActuallyMoving({0,0})&&kingdomActuallyMoving({.01f,0}));
 const sf::Vector2f foot{512.25f,408.75f};const auto pc=KingdomFootModel::playerCollider(foot),nc=KingdomFootModel::npcCollider(foot);for(int f=0;f<24;++f){assert(sameRect(pc,KingdomFootModel::playerCollider(foot)));assert(sameRect(nc,KingdomFootModel::npcCollider(foot)));}
 for(const auto&d:kingdomNpcRoutes()){for(auto p:d.route){if(!map.isNpcFootprintWalkable(KingdomFootModel::npcCollider(p)))std::cerr<<"unsafe route "<<p.x<<","<<p.y<<"\n";assert(map.isWalkable(p));assert(!map.isWater(p));assert(map.isNpcFootprintWalkable(KingdomFootModel::npcCollider(p)));}for(int phase=0;phase<4;++phase){auto p=kingdomPhaseDestination(d,static_cast<DayPhase>(phase));assert(map.isWalkable(p));assert(!map.isWater(p));assert(map.isNpcFootprintWalkable(KingdomFootModel::npcCollider(p)));}for(std::size_t i=1;i<d.route.size();++i)for(int s=0;s<=20;++s){auto p=d.route[i-1]+(d.route[i]-d.route[i-1])*(s/20.f);assert(map.isWalkable(p));assert(!map.isWater(p));}}
 assert(NpcScheduleSystem::activity(KingdomNpcRole::Citizen,DayPhase::Day)==NpcActivity::Market);assert(NpcScheduleSystem::activity(KingdomNpcRole::Guard,DayPhase::Night)==NpcActivity::GuardPost);
 assert(kingdomSortKey(400,1)>kingdomSortKey(900,0));for(std::size_t i=0;i<KINGDOM_OCCLUDER_KEYS.size();++i)for(std::size_t j=i+1;j<KINGDOM_OCCLUDER_KEYS.size();++j)assert(KINGDOM_OCCLUDER_KEYS[i]!=KINGDOM_OCCLUDER_KEYS[j]);assert(KINGDOM_OCCLUDER_KEYS[4]<KINGDOM_OCCLUDER_KEYS[5]);assert(KINGDOM_OCCLUDER_KEYS[2]<KINGDOM_OCCLUDER_KEYS[0]);
 DayNightSystem day;day.setNormalizedTimeForTesting(.7f);float strength=day.nightStrength();day.update(.01f);assert(std::abs(day.nightStrength()-strength)<.01f);auto phase=day.phase();day.advancePhase();assert(day.phase()!=phase);assert(kingdomRainRippleAllowed(map,{1300,520})&&!kingdomRainRippleAllowed(map,{700,500}));
 const auto root=std::filesystem::path(KINGDOM_SOURCE_DIR);sf::Image mask,flow;assert(mask.loadFromFile(root/"assets/images/kingdom/layers/kingdom_water_mask.png"));assert(flow.loadFromFile(root/"assets/images/kingdom/layers/kingdom_water_flow_0.png"));for(unsigned y=0;y<941;y+=7)for(unsigned x=0;x<1672;x+=7)if(flow.getPixel({x,y}).a)assert(mask.getPixel({x,y}).r>0);
 sf::Texture texture({1536,1024});KingdomNpc shore(KingdomNpcRole::Citizen,texture,{{1000,680},{1300,680}},90);auto shoreStart=shore.getPosition();for(int i=0;i<180;++i){shore.update(1.f/60,map);requireNpcSafe(map,shore);}shore.update(.25f,map);requireNpcSafe(map,shore);assert(std::hypot(shore.getPosition().x-shoreStart.x,shore.getPosition().y-shoreStart.y)>20);
 KingdomNpc left(KingdomNpcRole::Guard,texture,{{160,600}},72),right(KingdomNpcRole::Guard,texture,{{166,600}},72);float before=std::hypot(left.getPosition().x-right.getPosition().x,left.getPosition().y-right.getPosition().y);left.avoid(right.getPosition(),map,24);right.avoid(left.getPosition(),map,24);float after=std::hypot(left.getPosition().x-right.getPosition().x,left.getPosition().y-right.getPosition().y);assert(after>before);requireNpcSafe(map,left);requireNpcSafe(map,right);KingdomNpc ca(KingdomNpcRole::Guard,texture,{{160,600}},72),cb(KingdomNpcRole::Guard,texture,{{166,600}},72);assert(std::hypot(ca.getPosition().x-cb.getPosition().x,ca.getPosition().y-cb.getPosition().y)==before);
 KingdomCompositor pixels;auto dayImage=frame(pixels,{0,0,0,0},KingdomWeather::Clear),nightImage=frame(pixels,{18,45,87,120},KingdomWeather::Clear);assert(dayImage.getPixel({10,10})!=nightImage.getPixel({10,10}));assert(dayImage.getPixel({55,55})==nightImage.getPixel({55,55}));auto rain=frame(pixels,{0,0,0,0},KingdomWeather::LightRain),mist=frame(pixels,{0,0,0,0},KingdomWeather::Mist);bool rainChanged=false,mistChanged=false;for(unsigned y=0;y<48;++y)for(unsigned x=0;x<64;++x){rainChanged|=rain.getPixel({x,y})!=dayImage.getPixel({x,y});mistChanged|=mist.getPixel({x,y})!=dayImage.getPixel({x,y});}assert(rainChanged&&mistChanged);assert(rain.getPixel({55,55})==dayImage.getPixel({55,55}));assert(mist.getPixel({55,55})==dayImage.getPixel({55,55}));
 KingdomCompositor life;assert(life.ensure({48,48}));sf::View v(sf::FloatRect{{0,0},{48,48}});for(int i=0;i<120;++i){assert(life.ensure({48,48}));life.begin(v);life.finish();}assert(life.initializationCount()==1&&life.recreationCount()==0);assert(life.ensure({48,48})&&life.recreationCount()==0);assert(life.ensure({64,64})&&life.recreationCount()==1);for(int i=0;i<120;++i)assert(life.ensure({64,64}));assert(life.recreationCount()==1);
 return 0;
}
