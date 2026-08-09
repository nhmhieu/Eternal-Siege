#include "KingdomRenderer.h"
#include <cmath>
KingdomRenderer::KingdomRenderer(TextureManager&t){
 if(const auto*x=t.findTexture("KingdomWorldOpen"))base.emplace(*x);
 for(std::size_t i=0;i<2;++i)if(const auto*x=t.findTexture("KingdomWater"+std::to_string(i)))water[i].emplace(*x);
 const char*k[]={"KingdomFgGateLeft","KingdomFgGateRight","KingdomFgGateArch","KingdomFgFountain","KingdomFgBridgeFar","KingdomFgBridgeNear","KingdomFgCatacomb","KingdomFgTreeVillage","KingdomFgTreeBridge","KingdomFgTreeRiver"};
 for(std::size_t i=0;i<OCCLUDER_COUNT;++i)if(const auto*x=t.findTexture(k[i]))occluders[i].emplace(*x);
}
void KingdomRenderer::drawBase(sf::RenderTarget&w)const{if(base)w.draw(*base);}
void KingdomRenderer::drawWater(sf::RenderTarget&w,float time,float night)const{const float b=.5f+.5f*std::sin(time*.65f);for(std::size_t i=0;i<2;++i)if(water[i]){auto s=*water[i];const float a=(i?b:1-b)*(1-.28f*night);s.setColor({255,255,255,static_cast<std::uint8_t>(255*a)});w.draw(s);}}
void KingdomRenderer::drawOccluder(sf::RenderTarget&w,std::size_t i)const{if(i<OCCLUDER_COUNT&&occluders[i])w.draw(*occluders[i]);}
