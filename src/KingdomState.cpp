#include "KingdomState.h"
#include "LevelSelectState.h"
#include "MenuState.h"
#include "AssetLocator.h"
#include "KingdomAssets.h"
#include <algorithm>
#include <cmath>

KingdomState::KingdomState(StateMachine&m,sf::RenderWindow&w,TextureManager&t,AudioManager&a,GameProgress&p,bool back):machine(m),window(w),textures(t),audio(a),progress(p),gate(p.castleGateOpen),returning(back){}

void KingdomState::onEnter(){
 position=returning?KingdomMap::RETURN_SPAWN:KingdomMap::SPAWN;transitioning=false;transitionLatch.reset();fade=1;elapsed=promptAlpha=0;
 const auto path=AssetLocator::find("assets/fonts/Font.ttf");fontLoaded=path&&font.openFromFile(*path);audio.playMusic("assets/audio/music/menu_theme.ogg");
 preloadKingdomAssets(textures);renderer=std::make_unique<KingdomRenderer>(textures);compositor=std::make_unique<KingdomCompositor>();compositor->ensure({1280,720});
 avatar=std::make_unique<Player>(textures);if(const auto*s=textures.findTexture("PlayerMageWalk"))avatar->useWalkSpriteSheet(*s);avatar->setPosition(position);
 npcs.clear();for(const auto&definition:kingdomNpcRoutes()){const char*key=definition.role==KingdomNpcRole::Guard?"GuardWalk":"CitizenWalk";if(const auto*t=textures.findTexture(key))npcs.push_back(std::make_unique<KingdomNpc>(definition.role,*t,definition.route));}
 uiView=window.getDefaultView();worldView=sf::View(sf::FloatRect{{0,0},{1280,720}});cameraCenter=position;cameraCenter.x=std::clamp(cameraCenter.x,640.f,1032.f);cameraCenter.y=std::clamp(cameraCenter.y,360.f,581.f);worldView.setCenter(cameraCenter);
}
void KingdomState::onExit(){}
void KingdomState::handleEvent(const sf::Event&e){
 if(transitioning)return;if(const auto*k=e.getIf<sf::Event::KeyPressed>()){
  if(k->code==sf::Keyboard::Key::Escape){machine.changeState(std::make_unique<MenuState>(machine,window,textures,audio,progress));return;}
  if(k->code==sf::Keyboard::Key::F3){debugCollision=!debugCollision;return;}if(k->code==sf::Keyboard::Key::F4){dayNight.advancePhase();return;}if(k->code==sf::Keyboard::Key::F5){environment.advanceWeather();return;}if(k->code==sf::Keyboard::Key::F6){debugPerformance=!debugPerformance;return;}
  if(k->code==sf::Keyboard::Key::E){if(gate.tryOpen(map.canInteractWithGate(position))){audio.playSound("ui_click");return;}if(gate.getState()==GateState::Open&&map.canInteract(position)&&transitionLatch.request()){transitioning=true;audio.playSound("ui_click");}}
 }
}
void KingdomState::update(float dt){
 dt=std::min(dt,1.f/30.f);lastFrameDt=dt;elapsed+=dt;dayNight.update(dt);environment.update(dt);
 if(transitioning){fade=std::min(1.f,fade+dt/.32f);if(fade>=1)machine.changeState(std::make_unique<LevelSelectState>(machine,window,textures,audio,progress));return;}
 gate.update(dt);if(gate.getState()==GateState::Open)progress.castleGateOpen=true;fade=std::max(0.f,fade-dt/.32f);
 sf::Vector2f input;if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))input.y--;if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))input.y++;if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))input.x--;if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))input.x++;
 const float len=std::hypot(input.x,input.y);const auto before=position;if(len>0){input/=len;facing=input;position=map.resolveMovement(position,input*230.f*dt,14,!gate.blocksPassage());}const auto actual=position-before;if(avatar){avatar->setPosition(position);avatar->updateNonCombatPresentation(dt,actual);}
 for(auto&n:npcs){const auto activity=NpcScheduleSystem::activity(n->getRole(),dayNight.phase());n->setSchedulePaused(activity==NpcActivity::Rest);n->update(dt,map);}
 for(std::size_t i=0;i<npcs.size();++i){npcs[i]->avoid(position,map,28);for(std::size_t j=0;j<i;++j)npcs[i]->avoid(npcs[j]->getPosition(),map,24);}
 const sf::Vector2f lights[]={{84,875},{430,410},{845,520},{1300,205}};const float night=dayNight.nightStrength();auto tint=[&](sf::Vector2f p){float glow=0;for(auto l:lights)glow=std::max(glow,std::clamp(1.f-std::hypot(p.x-l.x,p.y-l.y)/145.f,0.f,1.f));return sf::Color(std::uint8_t(225-50*night+45*glow*night),std::uint8_t(228-48*night+32*glow*night),std::uint8_t(232-22*night));};if(avatar)avatar->setPresentationTint(tint(position));for(auto&n:npcs)n->setTint(tint(n->getPosition()));
 auto target=position;const auto offset=target-cameraCenter;if(std::abs(offset.x)<48)target.x=cameraCenter.x;if(std::abs(offset.y)<34)target.y=cameraCenter.y;cameraCenter+=(target-cameraCenter)*(1-std::exp(-7*dt));cameraCenter.x=std::clamp(cameraCenter.x,640.f,1032.f);cameraCenter.y=std::clamp(cameraCenter.y,360.f,581.f);worldView.setCenter(cameraCenter);
 const bool prompt=(gate.getState()==GateState::Closed&&map.canInteractWithGate(position))||(gate.getState()==GateState::Open&&map.canInteract(position));promptAlpha+=std::clamp((prompt?1.f:0.f)-promptAlpha,-dt*5,dt*5);
}
void KingdomState::drawText(sf::RenderWindow&w,const std::string&s,sf::Vector2f p,unsigned n,sf::Color c)const{if(fontLoaded){sf::Text t(font,s,n);t.setPosition(p);t.setFillColor(c);w.draw(t);}}
void KingdomState::render(sf::RenderWindow&w){
 if(!compositor)return;sceneTarget=&compositor->begin(worldView);if(renderer){renderer->drawBase(*sceneTarget);renderer->drawWater(*sceneTarget,elapsed,dayNight.nightStrength());}
 const float open=gate.openProgress(),eased=1-std::pow(1-open,3);if(const auto*door=textures.findTexture("CastleGate")){const auto size=door->getSize();const int half=int(size.x/2);sf::Sprite l(*door,{{0,0},{half,int(size.y)}}),r(*door,{{half,0},{int(size.x)-half,int(size.y)}});const float scale=112.f/size.x;l.setScale({scale,scale});r.setScale({scale,scale});l.setPosition({164-55*eased,748});r.setPosition({220+55*eased,748});sceneTarget->draw(l);sceneTarget->draw(r);}
 struct Item{float key;int kind;std::size_t index;};std::vector<Item>items{{position.y+map.heightAt(position)*1000,0,0}};for(std::size_t i=0;i<npcs.size();++i)items.push_back({npcs[i]->getSortKey()+map.heightAt(npcs[i]->getPosition())*1000,1,i});if(renderer)for(std::size_t i=0;i<KingdomRenderer::OCCLUDER_COUNT;++i)items.push_back({renderer->occluderSortKey(i),2,i});std::stable_sort(items.begin(),items.end(),[](auto&a,auto&b){return a.key<b.key;});
 auto shadow=[&](sf::Vector2f p,float scale){if(map.isWater(p))return;if(const auto*t=textures.findTexture("KingdomContactShadow")){sf::Sprite s(*t);s.setOrigin({64,32});s.setPosition(p);s.setScale({scale,scale});s.setColor({255,255,255,std::uint8_t(220-70*dayNight.nightStrength())});sceneTarget->draw(s);}};
 for(auto&i:items){if(i.kind==0&&avatar){shadow(position,.48f);avatar->draw(*sceneTarget);}else if(i.kind==1){shadow(npcs[i.index]->getPosition(),.42f);npcs[i.index]->draw(*sceneTarget);}else if(renderer)renderer->drawOccluder(*sceneTarget,i.index);}
 if(environment.getWeather()==KingdomWeather::LightRain)for(int i=0;i<18;++i){sf::Vector2f p{std::fmod(i*137.f+elapsed*41.f,KingdomMap::WORLD_SIZE.x),std::fmod(i*83.f+elapsed*27.f,KingdomMap::WORLD_SIZE.y)};if(map.isWater(p)){sf::CircleShape ripple(5);ripple.setOrigin({5,5});ripple.setScale({1,.35f});ripple.setPosition(p);ripple.setFillColor(sf::Color::Transparent);ripple.setOutlineThickness(1);ripple.setOutlineColor({190,225,235,75});sceneTarget->draw(ripple);}}
 compositor->applyAmbient(dayNight.ambientOverlay());if(dayNight.nightStrength()>.01f)for(std::size_t i=0;i<4;++i){const sf::Vector2f p=std::array<sf::Vector2f,4>{{{84,875},{430,410},{845,520},{1300,205}}}[i];sf::CircleShape glow(110);glow.setOrigin({110,110});glow.setPosition(p);const float flicker=.92f+.08f*std::sin(elapsed*3.1f+i*2.7f);glow.setFillColor({255,175,70,std::uint8_t(34*dayNight.nightStrength()*flicker)});sceneTarget->draw(glow,sf::BlendAdd);}
 if(debugCollision){
  auto rectangle=[&](sf::FloatRect r,sf::Color fill,sf::Color outline){sf::RectangleShape q(r.size);q.setPosition(r.position);q.setFillColor(fill);q.setOutlineColor(outline);q.setOutlineThickness(2);sceneTarget->draw(q);};
  for(const auto&w:map.getWaterZones())rectangle(w,{25,90,235,45},{55,155,255,180});
  for(const auto&f:map.getSolidFootprints())rectangle(f.bounds,{235,45,45,62},{255,95,70,215});
  const sf::Vector2f a{1030,655},b{1390,880},ab=b-a;const float length=std::hypot(ab.x,ab.y);const sf::Vector2f n{-ab.y/length,ab.x/length};
  sf::VertexArray deck(sf::PrimitiveType::TriangleStrip,4);deck[0].position=a-n*50.f;deck[1].position=a+n*50.f;deck[2].position=b-n*50.f;deck[3].position=b+n*50.f;for(std::size_t i=0;i<4;++i)deck[i].color={30,225,190,65};sceneTarget->draw(deck);
  for(float side:{-61.f,61.f}){sf::VertexArray rail(sf::PrimitiveType::Lines,2);rail[0].position=a+n*side;rail[1].position=b+n*side;rail[0].color=rail[1].color={255,75,65,235};sceneTarget->draw(rail);}
  rectangle({{158,830},{132,87}},{60,230,110,40},{80,255,145,205});
  auto marker=[&](sf::Vector2f p,float radius,sf::Color color){sf::CircleShape c(radius);c.setOrigin({radius,radius});c.setPosition(p);c.setFillColor(color);sceneTarget->draw(c);};marker(KingdomMap::SPAWN,7,{255,220,35,245});marker(position,5,{255,255,255,250});
  const auto foot=avatar?avatar->getFootCollider():KingdomFootModel::playerCollider(position);rectangle(foot,{255,240,45,85},{255,245,100,245});
  if(renderer)for(std::size_t i=0;i<KingdomRenderer::OCCLUDER_COUNT;++i){const float y=renderer->occluderSortKey(i);sf::VertexArray baseline(sf::PrimitiveType::Lines,2);baseline[0].position={0,y};baseline[1].position={KingdomMap::WORLD_SIZE.x,y};baseline[0].color=baseline[1].color={215,85,255,90};sceneTarget->draw(baseline);}
 }
 compositor->finish();w.clear();w.setView(uiView);compositor->drawWorld(w);compositor->drawWeather(w,environment.getWeather(),elapsed);
 sf::RectangleShape objective({370,76});objective.setPosition({24,24});objective.setFillColor({5,14,22,205});objective.setOutlineColor({226,184,92,190});objective.setOutlineThickness(1);w.draw(objective);drawText(w,"OBJECTIVE",{44,34},15,{238,201,112});drawText(w,gate.getState()==GateState::Open?"Reach the Ruined Catacombs":"Open the Castle Gate",{44,61},21,{235,241,234});drawText(w,"WASD Move    E Interact    F3 Navigation    F4 Time    F5 Weather    F6 Performance",{28,684},14,{220,224,215,205});
 if(debugCollision)drawText(w,"F3 CALIBRATION  foot ("+std::to_string(int(position.x))+", "+std::to_string(int(position.y))+")  yellow=foot/spawn  red=solid/rail  blue=water  cyan=deck  green=entrance",{405,30},14,{255,245,190});
 if(debugPerformance){const char*phases[]={"Dawn","Day","Sunset","Night"};const char*weather[]={"Clear","Rain","Mist"};drawText(w,"FPS "+std::to_string(int(lastFrameDt>0?1/lastFrameDt:0))+"  frame "+std::to_string(lastFrameDt*1000).substr(0,5)+" ms\nphase "+phases[int(dayNight.phase())]+"  weather "+weather[int(environment.getWeather())]+"\ncache "+std::to_string(textures.getCacheHits())+" hit / "+std::to_string(textures.getCacheMisses())+" miss",{930,28},14,{235,240,225});}
 if(promptAlpha>0){sf::RectangleShape panel({330,44});panel.setPosition({475,646});panel.setFillColor({6,15,22,std::uint8_t(205*promptAlpha)});w.draw(panel);drawText(w,gate.getState()==GateState::Closed?"E   Open the Gate":"E   Enter Ruined Catacombs",{492,656},17,{240,239,226,std::uint8_t(255*promptAlpha)});}sf::RectangleShape veil({1280,720});veil.setFillColor({3,7,12,std::uint8_t(255*std::clamp(fade,0.f,1.f))});w.draw(veil);
}
