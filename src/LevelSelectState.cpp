#include "LevelSelectState.h"
#include "SetupState.h"
#include "KingdomState.h"
#include "AssetLocator.h"

#include <string>

namespace {
sf::FloatRect cardRect(std::size_t i) {
    return {{70.f + static_cast<float>(i) * 390.f, 178.f}, {350.f, 330.f}};
}
void centerText(sf::Text& text, sf::Vector2f center) {
    const auto b = text.getLocalBounds();
    text.setOrigin({b.position.x + b.size.x / 2.f,
                    b.position.y + b.size.y / 2.f});
    text.setPosition(center);
}
}

LevelSelectState::LevelSelectState(StateMachine& m, sf::RenderWindow& w,
 TextureManager& t, AudioManager& a, GameProgress& p)
 : machine(m),window(w),textures(t),audio(a),progress(p) {}

void LevelSelectState::onEnter(){selected=0;transition=false;const auto p=AssetLocator::find("assets/fonts/Font.ttf");fontLoaded=p&&font.openFromFile(*p);}
void LevelSelectState::onExit(){}
void LevelSelectState::confirm(){if(transition||!LEVEL_DEFINITIONS[selected].unlocked)return;transition=true;audio.playSound("ui_click");machine.changeState(std::make_unique<SetupState>(machine,window,textures,audio,progress,LEVEL_DEFINITIONS[selected].id));}
void LevelSelectState::handleEvent(const sf::Event& e){if(transition)return;if(const auto*k=e.getIf<sf::Event::KeyPressed>()){if(k->code==sf::Keyboard::Key::Escape){machine.changeState(std::make_unique<KingdomState>(machine,window,textures,audio,progress,true));return;}if(k->code==sf::Keyboard::Key::A||k->code==sf::Keyboard::Key::Left)selected=(selected+LEVEL_DEFINITIONS.size()-1)%LEVEL_DEFINITIONS.size();if(k->code==sf::Keyboard::Key::D||k->code==sf::Keyboard::Key::Right)selected=(selected+1)%LEVEL_DEFINITIONS.size();if(k->code==sf::Keyboard::Key::Enter)confirm();}if(const auto*m=e.getIf<sf::Event::MouseButtonPressed>();m&&m->button==sf::Mouse::Button::Left){const auto p=window.mapPixelToCoords(m->position);for(std::size_t i=0;i<LEVEL_DEFINITIONS.size();++i)if(cardRect(i).contains(p)){selected=i;confirm();}}}
void LevelSelectState::update(float){const auto p=window.mapPixelToCoords(sf::Mouse::getPosition(window));for(std::size_t i=0;i<LEVEL_DEFINITIONS.size();++i)if(cardRect(i).contains(p))selected=i;}

void LevelSelectState::render(sf::RenderWindow& w) {
    w.setView(w.getDefaultView()); w.clear({5,13,22});
    if(!fontLoaded)return;
    auto text=[&](const std::string& s,sf::Vector2f p,unsigned z,sf::Color c,bool centered=false){sf::Text t(font,s,z);t.setFillColor(c);if(centered)centerText(t,p);else t.setPosition(p);w.draw(t);};
    text("SELECT A DUNGEON",{48,40},36,{240,202,105});
    sf::RectangleShape wallet({190,54}); wallet.setPosition({1042,34});wallet.setFillColor({12,29,38,235});wallet.setOutlineColor({217,170,73,190});wallet.setOutlineThickness(1.5f);w.draw(wallet);
    text("GOLD  " + std::to_string(progress.totalGold),{1137,61},21,{245,210,112},true);
    for(std::size_t i=0;i<LEVEL_DEFINITIONS.size();++i){const auto r=cardRect(i);sf::RectangleShape card(r.size);card.setPosition(r.position);card.setFillColor(LEVEL_DEFINITIONS[i].unlocked?sf::Color(15,38,46):sf::Color(25,28,34));card.setOutlineThickness(i==selected?4.f:2.f);card.setOutlineColor(i==selected?sf::Color(72,228,197):sf::Color(75,85,90));w.draw(card);text(std::string(LEVEL_DEFINITIONS[i].title),{r.position.x+20,r.position.y+30},18,LEVEL_DEFINITIONS[i].unlocked?sf::Color(245,213,130):sf::Color(120,125,130));text(std::string(LEVEL_DEFINITIONS[i].description),{r.position.x+20,r.position.y+92},16,{175,190,193});const bool cleared=i==0&&progress.ruinedCatacombsCleared;text(cleared?"CLEARED":(LEVEL_DEFINITIONS[i].unlocked?"AVAILABLE":"LOCKED"),{r.position.x+175,r.position.y+278},17,cleared?sf::Color(245,201,92):(LEVEL_DEFINITIONS[i].unlocked?sf::Color(95,225,180):sf::Color(145,90,90)),true);}
    const auto& level=LEVEL_DEFINITIONS[selected];text(std::string(level.objective),{48,550},19,{210,222,218});text("A/D or Arrows  Select    ENTER  Confirm    ESC  Kingdom",{48,670},16,{170,190,194});
}
