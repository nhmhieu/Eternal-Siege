#include "KingdomState.h"
#include "LevelSelectState.h"
#include "MenuState.h"
#include "AssetLocator.h"
#include "KingdomAssets.h"
#include <algorithm>
#include <cmath>

KingdomState::KingdomState(StateMachine& m, sf::RenderWindow& w,
 TextureManager& t, AudioManager& a, GameProgress& gameProgress, bool back)
 : machine(m),window(w),textures(t),audio(a),progress(gameProgress),
   gate(gameProgress.castleGateOpen),returning(back) {}

void KingdomState::onEnter(){
 position=returning?KingdomMap::RETURN_SPAWN:KingdomMap::SPAWN; transitioning=false; fade=1.f; elapsed=0.f; promptAlpha=0.f;
 const auto p=AssetLocator::find("assets/fonts/Font.ttf"); fontLoaded=p&&font.openFromFile(*p); audio.playMusic("assets/audio/music/menu_theme.ogg");
 preloadKingdomAssets(textures);
 if(const auto* texture=textures.findTexture("KingdomWorldOpen")) worldSprite.emplace(*texture); else worldSprite.reset();
 avatar=std::make_unique<Player>(textures);if(const auto* sheet=textures.findTexture("PlayerMageWalk"))avatar->useWalkSpriteSheet(*sheet);avatar->setPosition(position);
 npcs.clear();if(const auto*g=textures.findTexture("GuardWalk")){npcs.push_back(std::make_unique<KingdomNpc>(KingdomNpcRole::Guard,*g,std::vector<sf::Vector2f>{{125,850}}));npcs.push_back(std::make_unique<KingdomNpc>(KingdomNpcRole::Guard,*g,std::vector<sf::Vector2f>{{320,850}}));npcs.push_back(std::make_unique<KingdomNpc>(KingdomNpcRole::Guard,*g,std::vector<sf::Vector2f>{{430,720},{610,650},{520,570}}));npcs.push_back(std::make_unique<KingdomNpc>(KingdomNpcRole::Guard,*g,std::vector<sf::Vector2f>{{1260,330},{1360,300}}));}if(const auto*c=textures.findTexture("CitizenWalk")){npcs.push_back(std::make_unique<KingdomNpc>(KingdomNpcRole::Citizen,*c,std::vector<sf::Vector2f>{{520,470},{700,450},{650,570}}));npcs.push_back(std::make_unique<KingdomNpc>(KingdomNpcRole::Citizen,*c,std::vector<sf::Vector2f>{{420,390},{540,350}}));npcs.push_back(std::make_unique<KingdomNpc>(KingdomNpcRole::Citizen,*c,std::vector<sf::Vector2f>{{760,310},{900,350},{820,450}}));npcs.push_back(std::make_unique<KingdomNpc>(KingdomNpcRole::Citizen,*c,std::vector<sf::Vector2f>{{470,180},{610,190}}));}
 uiView=window.getDefaultView(); worldView=sf::View(sf::FloatRect{{0.f,0.f},{1280.f,720.f}});
 cameraCenter=position; cameraCenter.x=std::clamp(cameraCenter.x,640.f,KingdomMap::WORLD_SIZE.x-640.f); cameraCenter.y=std::clamp(cameraCenter.y,360.f,KingdomMap::WORLD_SIZE.y-360.f); worldView.setCenter(cameraCenter);
}
void KingdomState::onExit(){}
void KingdomState::handleEvent(const sf::Event& e){
 if(transitioning)return;
 if(const auto* k=e.getIf<sf::Event::KeyPressed>()){
  if(k->code==sf::Keyboard::Key::Escape){ machine.changeState(std::make_unique<MenuState>(machine,window,textures,audio,progress)); return; }
  if(k->code==sf::Keyboard::Key::F3){debugCollision=!debugCollision;return;}
  if(k->code==sf::Keyboard::Key::F4){dayNight.advancePhase();return;}
  if(k->code==sf::Keyboard::Key::E){
   if(gate.tryOpen(map.canInteractWithGate(position))){audio.playSound("ui_click");return;}
   if(gate.getState()==GateState::Open && map.canInteract(position)){ transitioning=true; audio.playSound("ui_click"); }
  }
 }
}
void KingdomState::update(float dt){
 dt=std::min(dt,1.f/30.f); elapsed+=dt; dayNight.update(dt);
 if(transitioning){ fade=std::min(1.f,fade+dt/0.32f); if(fade>=1.f) machine.changeState(std::make_unique<LevelSelectState>(machine,window,textures,audio,progress)); return; }
 gate.update(dt); if(gate.getState()==GateState::Open)progress.castleGateOpen=true;
 fade=std::max(0.f,fade-dt/0.32f);
 sf::Vector2f d;
 if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))d.y-=1;
 if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))d.y+=1;
 if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))d.x-=1;
 if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))d.x+=1;
 const float len=std::sqrt(d.x*d.x+d.y*d.y); if(len>0){d/=len;facing=d;position=map.resolveMovement(position,d*230.f*dt,18.f,!gate.blocksPassage());}
 if(avatar){avatar->setPosition(position);avatar->updateNonCombatPresentation(dt,d);}
 for(auto& npc:npcs)npc->update(dt,map);
 const sf::Vector2f lights[]={{84,875},{430,410},{845,520},{1300,205}};const float night=dayNight.nightStrength();auto tintAt=[&](sf::Vector2f p){float glow=0;for(const auto&l:lights){const float distance=std::hypot(p.x-l.x,p.y-l.y);glow=std::max(glow,std::clamp(1.f-distance/145.f,0.f,1.f));}return sf::Color(static_cast<std::uint8_t>(225-50*night+45*glow*night),static_cast<std::uint8_t>(228-48*night+32*glow*night),static_cast<std::uint8_t>(232-22*night));};if(avatar)avatar->setPresentationTint(tintAt(position));for(auto&npc:npcs)npc->setTint(tintAt(npc->getPosition()));
 sf::Vector2f cameraTarget=position; const sf::Vector2f offset=cameraTarget-cameraCenter;
 if(std::abs(offset.x)<48.f)cameraTarget.x=cameraCenter.x;if(std::abs(offset.y)<34.f)cameraTarget.y=cameraCenter.y;
 const float follow=1.f-std::exp(-7.f*dt);cameraCenter+=(cameraTarget-cameraCenter)*follow;
 cameraCenter.x=std::clamp(cameraCenter.x,640.f,KingdomMap::WORLD_SIZE.x-640.f);cameraCenter.y=std::clamp(cameraCenter.y,360.f,KingdomMap::WORLD_SIZE.y-360.f);worldView.setCenter(cameraCenter);
 const bool gatePrompt=gate.getState()==GateState::Closed&&map.canInteractWithGate(position);
 const bool cavePrompt=gate.getState()==GateState::Open&&map.canInteract(position);
 const float promptTarget=(gatePrompt||cavePrompt)?1.f:0.f; promptAlpha+=std::clamp(promptTarget-promptAlpha,-dt*5.f,dt*5.f);
}
void KingdomState::drawText(sf::RenderWindow& w,const std::string& s,sf::Vector2f p,unsigned n,sf::Color c)const{if(!fontLoaded)return;sf::Text t(font,s,n);t.setPosition(p);t.setFillColor(c);w.draw(t);}
void KingdomState::render(sf::RenderWindow& w){
 w.clear({8,15,18});w.setView(worldView);if(worldSprite)w.draw(*worldSprite);
 for(const auto&zone:map.getWaterZones())for(int i=0;i<7;++i){const float y=zone.position.y+12.f+i*zone.size.y/7.f+std::sin(elapsed*.9f+i)*4.f;const float x=zone.position.x+18.f+std::fmod(elapsed*18.f+i*47.f,std::max(24.f,zone.size.x-55.f));if(map.isOnBridge({x,y},4.f))continue;sf::RectangleShape ripple({32.f+8.f*std::sin(elapsed+i),1.5f});ripple.setPosition({x,y});ripple.setFillColor({105,225,230,static_cast<std::uint8_t>(28+16*(.5f+.5f*std::sin(elapsed*1.7f+i)))});w.draw(ripple);}
 {const float t=gate.openProgress();const float eased=1.f-std::pow(1.f-t,3.f);if(const auto*door=textures.findTexture("CastleGate")){const auto size=door->getSize();const int half=static_cast<int>(size.x/2);sf::Sprite left(*door,{{0,0},{half,static_cast<int>(size.y)}}),right(*door,{{half,0},{static_cast<int>(size.x)-half,static_cast<int>(size.y)}});const float scale=112.f/static_cast<float>(size.x);left.setScale({scale,scale});right.setScale({scale,scale});left.setPosition({164.f-55.f*eased,748.f});right.setPosition({220.f+55.f*eased,748.f});w.draw(left);w.draw(right);}if(gate.getState()==GateState::Opening){for(int i=0;i<5;++i){sf::CircleShape dust(3.f+i*.4f);dust.setPosition({175.f+i*20.f,855.f-std::sin(elapsed*5.f+i)*4.f});dust.setFillColor({190,155,105,static_cast<std::uint8_t>(80*(1.f-t))});w.draw(dust);}}}
 const float pulse=.5f+.5f*std::sin(elapsed*2.f);if(gate.getState()==GateState::Open){sf::CircleShape caveGlow(64.f);caveGlow.setOrigin({64.f,64.f});caveGlow.setPosition(KingdomMap::CAVE_CENTER);caveGlow.setFillColor({60,225,205,static_cast<std::uint8_t>(12+18*pulse)});w.draw(caveGlow);
 for(const auto& point:KingdomMap::GOLDEN_ROUTE){const float dist=std::hypot(position.x-point.x,position.y-point.y);if(dist<65.f)continue;sf::CircleShape rune(7.f+2.f*pulse);rune.setOrigin({7.f,7.f});rune.setPosition(point);rune.setFillColor({245,190,70,static_cast<std::uint8_t>(85+55*pulse)});rune.setOutlineColor({65,220,195,150});rune.setOutlineThickness(2.f);w.draw(rune);}}
 if(debugCollision){for(int y=0;y<KingdomMap::GRID_HEIGHT;++y)for(int x=0;x<KingdomMap::GRID_WIDTH;++x){sf::RectangleShape cell({31,31});cell.setPosition({float(x*KingdomMap::CELL_SIZE),float(y*KingdomMap::CELL_SIZE)});cell.setFillColor(map.cellAt(x,y).walkable?sf::Color(45,210,90,35):sf::Color(230,45,45,55));w.draw(cell);}sf::CircleShape foot(18);foot.setOrigin({18,18});foot.setPosition(position);foot.setFillColor({245,220,60,100});w.draw(foot);sf::RectangleShape gateBox(KingdomMap::GATE_BLOCKER.size);gateBox.setPosition(KingdomMap::GATE_BLOCKER.position);gateBox.setFillColor({190,70,230,100});w.draw(gateBox);sf::CircleShape zone(KingdomMap::GATE_INTERACTION_RADIUS);zone.setOrigin({KingdomMap::GATE_INTERACTION_RADIUS,KingdomMap::GATE_INTERACTION_RADIUS});zone.setPosition(KingdomMap::GATE_CENTER);zone.setFillColor({70,200,240,45});w.draw(zone);for(const auto&npc:npcs)for(const auto&p:npc->getRoute()){sf::CircleShape marker(5);marker.setOrigin({5,5});marker.setPosition(p);marker.setFillColor({180,70,240,180});w.draw(marker);}}
 {sf::RectangleShape ambient(KingdomMap::WORLD_SIZE);ambient.setFillColor(dayNight.ambientOverlay());w.draw(ambient);}
 std::sort(npcs.begin(),npcs.end(),[](const auto&a,const auto&b){return a->footY()<b->footY();});for(const auto&npc:npcs)npc->drawShadow(w);if(avatar)avatar->drawShadow(w);bool playerDrawn=false;for(const auto&npc:npcs){if(!playerDrawn&&avatar&&npc->footY()>position.y){avatar->draw(w);playerDrawn=true;}npc->draw(w);}if(avatar&&!playerDrawn)avatar->draw(w);
 sf::RectangleShape arch({190,20});arch.setPosition({130,742});arch.setFillColor({68,63,55,230});w.draw(arch);
 for(int i=0;i<7;i++){sf::CircleShape mote(2.f);mote.setPosition({1120.f+i*73.f,220.f+std::sin(elapsed*.8f+i)*38.f});mote.setFillColor({120,238,195,110});w.draw(mote);}
 w.setView(uiView);sf::RectangleShape objective({370,76});objective.setPosition({24,24});objective.setFillColor({5,14,22,205});objective.setOutlineColor({226,184,92,190});objective.setOutlineThickness(1.f);w.draw(objective);drawText(w,"OBJECTIVE",{44,34},15,{238,201,112});drawText(w,gate.getState()==GateState::Open?"Reach the Ruined Catacombs":"Open the Castle Gate",{44,61},21,{235,241,234});
 drawText(w,"WASD Move    E Interact    F3 Navigation    F4 Time    ESC Menu",{28,684},14,{220,224,215,205});
 if(promptAlpha>0){const bool openingGate=gate.getState()==GateState::Closed;sf::RectangleShape panel({330,44});panel.setPosition({475,646});panel.setFillColor({6,15,22,static_cast<std::uint8_t>(205*promptAlpha)});panel.setOutlineColor({226,184,92,static_cast<std::uint8_t>(190*promptAlpha)});panel.setOutlineThickness(1.f);w.draw(panel);sf::RectangleShape keycap({34,30});keycap.setPosition({487,653});keycap.setFillColor({33,29,22,static_cast<std::uint8_t>(240*promptAlpha)});keycap.setOutlineColor({245,207,119,static_cast<std::uint8_t>(220*promptAlpha)});keycap.setOutlineThickness(1.f);w.draw(keycap);drawText(w,"E",{498,655},18,{255,228,150,static_cast<std::uint8_t>(255*promptAlpha)});drawText(w,openingGate?"Open the Gate":"Enter Ruined Catacombs",{536,656},17,{240,239,226,static_cast<std::uint8_t>(255*promptAlpha)});}
 sf::RectangleShape veil({1280,720});veil.setFillColor({3,7,12,static_cast<std::uint8_t>(255*std::clamp(fade,0.f,1.f))});w.draw(veil);
}
