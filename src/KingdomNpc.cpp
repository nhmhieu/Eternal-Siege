#include "KingdomNpc.h"
#include <cmath>
KingdomNpc::KingdomNpc(KingdomNpcRole r,const sf::Texture&t,std::vector<sf::Vector2f>path,float s):role(r),texture(&t),sprite(t),route(std::move(path)),speed(s){position=route.empty()?sf::Vector2f{}:route.front();sprite.setOrigin({128,236});sprite.setScale({.34f,.34f});setFacing(FacingDirection::Down);}
void KingdomNpc::setFacing(FacingDirection d){if(d==facing&&animation.currentFrame())return;facing=d;AnimationClip clip;for(int i=0;i<6;++i)clip.frames.push_back({{i*256,int(d)*256},{256,256}});animation.setClip(std::move(clip));}
void KingdomNpc::update(float dt,const KingdomMap&map){
 if(schedulePaused||route.size()<2||wait>0){wait=std::max(0.f,wait-dt);animation.reset();if(const auto*f=animation.currentFrame())sprite.setTextureRect(*f);sprite.setPosition(position);return;}
 const auto delta=route[target]-position;const float len=std::hypot(delta.x,delta.y);if(len<4){target=(target+1)%route.size();wait=.7f+float(target%3)*.35f;return;}const auto dir=delta/len;if(std::abs(dir.x)>std::abs(dir.y))setFacing(dir.x<0?FacingDirection::Left:FacingDirection::Right);else setFacing(dir.y<0?FacingDirection::Up:FacingDirection::Down);const auto next=map.resolveMovement(position,dir*std::min(speed*dt,len),12,true);const float moved=std::hypot(next.x-position.x,next.y-position.y);if(moved>.01f){position=next;animation.update(moved/speed);}else wait=.4f;if(const auto*f=animation.currentFrame())sprite.setTextureRect(*f);sprite.setPosition(position);
}
void KingdomNpc::avoid(sf::Vector2f other,const KingdomMap&map,float minimum){const auto d=position-other;const float length=std::hypot(d.x,d.y);if(length<.001f||length>=minimum)return;const auto candidate=map.resolveMovement(position,(d/length)*(minimum-length)*.35f,12,true);if(map.isNpcWalkable(candidate)){position=candidate;sprite.setPosition(position);}}
void KingdomNpc::drawShadow(sf::RenderWindow& w) const {
    sf::CircleShape ambient(14.f);
    ambient.setOrigin({14.f, 14.f});
    ambient.setScale({1.25f, 0.35f});
    ambient.setPosition({position.x, position.y + 1.f});
    ambient.setFillColor(sf::Color(6, 10, 16, 80));
    w.draw(ambient);

    sf::CircleShape contact(9.f);
    contact.setOrigin({9.f, 9.f});
    contact.setScale({1.1f, 0.28f});
    contact.setPosition({position.x, position.y + 2.f});
    contact.setFillColor(sf::Color(2, 4, 8, 130));
    w.draw(contact);
}
void KingdomNpc::draw(sf::RenderWindow& w) const { w.draw(sprite); }
void KingdomNpc::drawShadow(sf::RenderTexture& w) const {
    sf::CircleShape ambient(14.f);
    ambient.setOrigin({14.f, 14.f});
    ambient.setScale({1.25f, 0.35f});
    ambient.setPosition({position.x, position.y + 1.f});
    ambient.setFillColor(sf::Color(6, 10, 16, 75));
    w.draw(ambient);

    sf::CircleShape contact(9.f);
    contact.setOrigin({9.f, 9.f});
    contact.setScale({1.1f, 0.28f});
    contact.setPosition({position.x, position.y + 2.f});
    contact.setFillColor(sf::Color(2, 4, 8, 120));
    w.draw(contact);
}
void KingdomNpc::draw(sf::RenderTexture& w) const { w.draw(sprite); }
