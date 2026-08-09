#include "KingdomNpc.h"
#include <cmath>

KingdomNpc::KingdomNpc(KingdomNpcRole r,const sf::Texture& t,
 std::vector<sf::Vector2f> path,float s):role(r),texture(&t),sprite(t),route(std::move(path)),speed(s){position=route.empty()?sf::Vector2f{}:route.front();sprite.setOrigin({128,236});sprite.setScale({.34f,.34f});setFacing(FacingDirection::Down);}
void KingdomNpc::setFacing(FacingDirection d){if(d==facing&&animation.currentFrame())return;facing=d;AnimationClip clip;for(int i=0;i<6;++i)clip.frames.push_back({{i*256,static_cast<int>(d)*256},{256,256}});animation.setClip(std::move(clip));}
void KingdomNpc::update(float dt,const KingdomMap& map){if(route.size()<2||wait>0){wait=std::max(0.f,wait-dt);animation.reset();if(const auto*f=animation.currentFrame())sprite.setTextureRect(*f);sprite.setPosition(position);return;}const sf::Vector2f delta=route[target]-position;const float len=std::hypot(delta.x,delta.y);if(len<4){target=(target+1)%route.size();wait=.7f+static_cast<float>(target%3)*.35f;return;}const sf::Vector2f dir=delta/len;if(std::abs(dir.x)>std::abs(dir.y))setFacing(dir.x<0?FacingDirection::Left:FacingDirection::Right);else setFacing(dir.y<0?FacingDirection::Up:FacingDirection::Down);const auto next=map.resolveMovement(position,dir*std::min(speed*dt,len),12.f,true);if(std::hypot(next.x-position.x,next.y-position.y)>.01f){position=next;animation.update(dt);}else wait=.4f;if(const auto*f=animation.currentFrame())sprite.setTextureRect(*f);sprite.setPosition(position);}
void KingdomNpc::drawShadow(sf::RenderWindow&w)const{sf::CircleShape s(13);s.setOrigin({13,13});s.setScale({1.15f,.30f});s.setPosition({position.x,position.y+1});s.setFillColor({5,8,10,60});w.draw(s);}
void KingdomNpc::draw(sf::RenderWindow&w)const{w.draw(sprite);}
