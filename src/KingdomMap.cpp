#include "KingdomMap.h"
#include <algorithm>
#include <cmath>

KingdomMap::KingdomMap(){
 // All authored values are artwork pixels in the same 1672x941 world space.
 // Only ground-contact footprints are solid; roofs and tree crowns are depth layers.
 solidFootprints={
  {{{0,0},{26,941}},"west world edge"},{{{1646,0},{26,941}},"east world edge"},
  {{{0,0},{1672,24}},"north world edge"},{{{0,917},{1672,24}},"south world edge"},
  {{{34,793},{118,124}},"left gate pillar"},{{{298,783},{112,134}},"right gate pillar"},
  {{{0,884},{154,33}},"left gate wall"},{{{404,891},{238,26}},"right gate wall"},
  {{{37,176},{122,48}},"northwest cottage foundation"},{{{210,166},{143,47}},"north cottage foundation"},
  {{{386,175},{154,45}},"northeast cottage foundation"},
  {{{18,444},{170,65}},"tavern foundation"},{{{197,408},{244,75}},"guildhall foundation"},
  {{{661,376},{137,67}},"fountain basin"},
  {{{471,255},{31,34}},"village tree trunk"},{{{1128,589},{34,39}},"bridge tree trunk"},
  {{{1480,626},{38,43}},"river tree trunk"},{{{846,279},{25,31}},"square tree trunk"},
  {{{965,302},{25,30}},"east square tree trunk"},
  {{{1218,24},{164,225}},"catacomb west wall"},{{{1512,24},{134,310}},"catacomb east wall"},
  {{{1218,24},{428,73}},"catacomb upper wall"},{{{1228,286},{137,46}},"catacomb west buttress"},
  {{{1510,302},{136,49}},"catacomb east buttress"},{{{1372,316},{135,28}},"catacomb stair side"},
  {{{1090,371},{55,42}},"river cliff west"},{{{1568,387},{78,48}},"river cliff east"}
 };
 obstacles.reserve(solidFootprints.size());for(const auto&f:solidFootprints)obstacles.push_back(f.bounds);
 waterZones={{{865,785},{335,132}},{{1090,690},{330,227}},{{1180,410},{255,350}},{{1390,360},{256,557}}};
 cells.resize(GRID_WIDTH*GRID_HEIGHT);
 for(int y=0;y<GRID_HEIGHT;++y)for(int x=0;x<GRID_WIDTH;++x){
  const sf::Vector2f p{(x+.5f)*CELL_SIZE,(y+.5f)*CELL_SIZE};auto&c=cells[y*GRID_WIDTH+x];
  if(isOnBridge(p)){c.surface=KingdomSurface::Bridge;c.heightLevel=1;c.playerWalkable=c.npcWalkable=true;}
  else if(blocked(p,8,true)){c.playerWalkable=c.npcWalkable=false;c.surface=isWater(p)?KingdomSurface::Water:KingdomSurface::Blocked;}
  else c.surface=KingdomSurface::Road;
 }
}
const KingdomCell&KingdomMap::cellAt(int x,int y)const{static const KingdomCell out{KingdomSurface::Cliff,false,false,0};if(x<0||y<0||x>=GRID_WIDTH||y>=GRID_HEIGHT)return out;return cells[y*GRID_WIDTH+x];}
bool KingdomMap::isWalkable(sf::Vector2f p)const{if(p.x<0||p.y<0||p.x>=WORLD_SIZE.x||p.y>=WORLD_SIZE.y)return false;return!blocked(p,8,true);}
bool KingdomMap::isNpcWalkable(sf::Vector2f p)const{if(p.x<0||p.y<0||p.x>=WORLD_SIZE.x||p.y>=WORLD_SIZE.y)return false;return!blocked(p,8,true);}
bool KingdomMap::isWater(sf::Vector2f p)const{if(isOnBridge(p))return false;for(const auto&w:waterZones)if(w.contains(p))return true;return false;}
float KingdomMap::heightAt(sf::Vector2f p)const{return p.x<0||p.y<0?0:cellAt(int(p.x)/CELL_SIZE,int(p.y)/CELL_SIZE).heightLevel;}
bool KingdomMap::isOnBridge(sf::Vector2f p,float r)const{const sf::Vector2f a{1030,655},b{1390,880},ab=b-a,ap=p-a;const float t=std::clamp((ap.x*ab.x+ap.y*ab.y)/(ab.x*ab.x+ab.y*ab.y),0.f,1.f);const auto d=p-(a+ab*t);return d.x*d.x+d.y*d.y<=std::pow(std::max(0.f,58-r),2);}
bool KingdomMap::blocked(sf::Vector2f p,float r,bool gateOpen)const{auto circleHits=[&](const sf::FloatRect&q){const float x=std::clamp(p.x,q.position.x,q.position.x+q.size.x),y=std::clamp(p.y,q.position.y,q.position.y+q.size.y);const float dx=p.x-x,dy=p.y-y;return dx*dx+dy*dy<=r*r;};for(const auto&o:obstacles)if(circleHits(o))return true;
 // The deck wins over water, but its two stone rails remain solid footprints.
 const sf::Vector2f a{1030,655},b{1390,880},ab=b-a;const float length=std::hypot(ab.x,ab.y);const sf::Vector2f normal{-ab.y/length,ab.x/length};
 auto nearRail=[&](float side){const auto ra=a+normal*side,rb=b+normal*side,rab=rb-ra,ap=p-ra;const float t=std::clamp((ap.x*rab.x+ap.y*rab.y)/(rab.x*rab.x+rab.y*rab.y),0.f,1.f);return std::hypot((p-(ra+rab*t)).x,(p-(ra+rab*t)).y)<r+10.f;};
 if(nearRail(-61.f)||nearRail(61.f))return true;
 if(!isOnBridge(p,r))for(const auto&w:waterZones)if(circleHits(w))return true;if(!gateOpen&&circleHits(GATE_BLOCKER))return true;return false;}
sf::Vector2f KingdomMap::resolveMovement(sf::Vector2f p,sf::Vector2f d,float r,bool gateOpen)const{const int steps=std::max(1,int(std::ceil(std::hypot(d.x,d.y)/10)));const auto step=d/float(steps);for(int i=0;i<steps;++i){const sf::Vector2f x{p.x+step.x,p.y};if(!blocked(x,r,gateOpen))p.x=x.x;const sf::Vector2f y{p.x,p.y+step.y};if(!blocked(y,r,gateOpen))p.y=y.y;}return p;}
bool KingdomMap::isBlocked(sf::Vector2f p,float r,bool gateOpen)const{return blocked(p,r,gateOpen);}
bool KingdomMap::canInteract(sf::Vector2f p)const{return std::hypot(p.x-CAVE_CENTER.x,p.y-CAVE_CENTER.y)<=INTERACTION_RADIUS;}
bool KingdomMap::canInteractWithGate(sf::Vector2f p)const{return std::hypot(p.x-GATE_CENTER.x,p.y-GATE_CENTER.y)<=GATE_INTERACTION_RADIUS;}
bool KingdomMap::isNpcFootprintWalkable(sf::FloatRect b)const{const auto c=b.position+b.size/2.f;const float radius=std::max(b.size.x,b.size.y)*.5f;return c.x>=0&&c.y>=0&&c.x<WORLD_SIZE.x&&c.y<WORLD_SIZE.y&&!blocked(c,radius,true);}
