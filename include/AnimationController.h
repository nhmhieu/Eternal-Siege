#pragma once
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <vector>

enum class AnimationState { Idle, Walk, Attack, Hit, Death };
enum class FacingDirection { Down, Left, Right, Up };
struct AnimationClip { std::vector<sf::IntRect> frames; float frameDuration=.11f; bool loop=true; };

class AnimationController {
public:
    void setClip(AnimationClip value){clip=std::move(value);time=0.f;frame=0;}
    void update(float dt){if(clip.frames.empty())return;time+=std::max(0.f,dt);while(time>=clip.frameDuration){time-=clip.frameDuration;if(frame+1<clip.frames.size())++frame;else frame=clip.loop?0:clip.frames.size()-1;}}
    void reset(){time=0.f;frame=0;}
    const sf::IntRect* currentFrame()const{return clip.frames.empty()?nullptr:&clip.frames[frame];}
private: AnimationClip clip; float time=0.f; std::size_t frame=0;
};
