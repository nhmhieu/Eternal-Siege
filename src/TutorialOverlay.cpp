#include "TutorialOverlay.h"
#include "AssetLocator.h"
#include <algorithm>

void TutorialController::resetForNewGame(){mode=TutorialMode::FirstLaunch;enterHeld=false;helpHeld=false;awaitingInputRelease=false;}
TutorialTransition TutorialController::handleKeyPressed(TutorialKey key){if(awaitingInputRelease)return TutorialTransition::Consumed;if(key==TutorialKey::Enter){if(enterHeld)return TutorialTransition::Consumed;enterHeld=true;if(mode==TutorialMode::Hidden)return TutorialTransition::Ignored;const bool initial=mode==TutorialMode::FirstLaunch;mode=TutorialMode::Hidden;awaitingInputRelease=true;return initial?TutorialTransition::InitialClosed:TutorialTransition::Closed;}if(helpHeld)return TutorialTransition::Consumed;helpHeld=true;if(mode==TutorialMode::FirstLaunch)return TutorialTransition::Consumed;if(mode==TutorialMode::Reopened){mode=TutorialMode::Hidden;awaitingInputRelease=true;return TutorialTransition::Closed;}mode=TutorialMode::Reopened;return TutorialTransition::Reopened;}
bool TutorialController::handleKeyReleased(TutorialKey key){bool&held=key==TutorialKey::Enter?enterHeld:helpHeld;const bool consumed=held;held=false;return consumed;}
bool TutorialOverlay::load(){const auto path=AssetLocator::find("assets/fonts/Font.ttf");fontLoaded=path&&font.openFromFile(*path);return fontLoaded;}

void TutorialOverlay::draw(sf::RenderWindow& target,TutorialMode mode)const{
 if(mode==TutorialMode::Hidden)return;const sf::View view=target.getView();const sf::Vector2f size=view.getSize();const sf::Vector2f origin=view.getCenter()-size/2.f;
 sf::RectangleShape veil(size);veil.setPosition(origin);veil.setFillColor({3,8,16,220});target.draw(veil);
 const float scale=std::min(size.x/1280.f,size.y/720.f);const sf::Vector2f panelSize{std::min(1000.f,size.x-48.f),std::min(650.f,size.y-48.f)};const sf::Vector2f p=view.getCenter()-panelSize/2.f;
 sf::RectangleShape panel(panelSize);panel.setPosition(p);panel.setFillColor({10,25,35,248});panel.setOutlineColor({220,174,76,225});panel.setOutlineThickness(2.f);target.draw(panel);if(!fontLoaded)return;
 auto draw=[&](const std::string&s,sf::Vector2f at,unsigned z,sf::Color c,bool bold=false){sf::Text t(font,s,std::max(15u,static_cast<unsigned>(z*std::max(.82f,scale))));t.setPosition(at);t.setFillColor(c);if(bold)t.setStyle(sf::Text::Bold);target.draw(t);};
 auto centered=[&](const std::string&s,sf::Vector2f at,unsigned z,sf::Color c){sf::Text t(font,s,std::max(15u,static_cast<unsigned>(z*std::max(.82f,scale))));t.setFillColor(c);t.setStyle(sf::Text::Bold);const auto b=t.getLocalBounds();t.setOrigin({b.position.x+b.size.x/2.f,b.position.y+b.size.y/2.f});t.setPosition(at);target.draw(t);};
 const sf::Color gold{242,202,105},teal{99,231,207},body{224,232,232},muted{163,182,188};
 centered("HOW TO PLAY",{view.getCenter().x,p.y+42},34,gold);centered("Defend your team through 4 waves and defeat the final Boss.",{view.getCenter().x,p.y+82},17,body);
 sf::RectangleShape divider({panelSize.x-64.f,2.f});divider.setPosition({p.x+32,p.y+105});divider.setFillColor({65,211,190,150});target.draw(divider);
 const float left=p.x+54,right=p.x+panelSize.x*.56f,top=p.y+132,row=37.f;
 draw("MOVEMENT & COMBAT",{left,top},19,gold,true);draw("ABILITIES & SYSTEM",{right,top},19,gold,true);
 const char* lk[]={"WASD","Mouse","LMB Tap","Hold LMB","RMB / Shift"};const char* la[]={"Move","Aim","Spirit Bolt","Charged Attack","Dash"};
 const char* rk[]={"Q","P","M","R"};const char* ra[]={"Radiant Pulse","Pause","Toggle Audio","Show This Guide"};
 for(int i=0;i<5;++i){const float y=top+42+i*row;draw(lk[i],{left,y},16,teal,true);draw(la[i],{left+135,y},16,body);if(i<4){draw(rk[i],{right,y},16,teal,true);draw(ra[i],{right+70,y},16,body);}}
 const float partyY=top+250;sf::RectangleShape party({panelSize.x-108.f,105.f});party.setPosition({p.x+54,partyY});party.setFillColor({14,38,45,245});party.setOutlineColor({74,224,190,150});party.setOutlineThickness(1.5f);target.draw(party);draw("YOUR PARTY",{p.x+76,partyY+16},17,gold,true);draw("Your allies fight automatically.",{p.x+76,partyY+45},16,body,true);draw("Survive, earn Gold and upgrade between waves.",{p.x+76,partyY+72},15,muted);
 centered(mode==TutorialMode::FirstLaunch?"Press ENTER to begin":"Press R or ENTER to resume",{view.getCenter().x,p.y+panelSize.y-35},20,gold);
}
