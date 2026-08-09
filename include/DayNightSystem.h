#pragma once
#include <SFML/Graphics/Color.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
enum class DayPhase { Dawn,Day,Sunset,Night };
class DayNightSystem {
public:
 static constexpr float CYCLE_SECONDS=180.f;
 void update(float dt){time=std::fmod(time+std::max(0.f,dt),CYCLE_SECONDS);}
 void advancePhase(){time=((static_cast<int>(phase())+1)%4)*(CYCLE_SECONDS/4.f);}
 DayPhase phase()const{return static_cast<DayPhase>(std::min(3,static_cast<int>(time/(CYCLE_SECONDS/4.f))));}
 float normalizedTime()const{return time/CYCLE_SECONDS;}
 float nightStrength()const{const float p=normalizedTime();if(p<.5f)return 0;if(p<.75f)return(p-.5f)*4;return std::clamp(1-(p-.75f)*4,0.f,1.f);}
 sf::Color ambientOverlay()const{const float n=nightStrength();return{18,static_cast<std::uint8_t>(24+25*n),static_cast<std::uint8_t>(45+42*n),static_cast<std::uint8_t>(12+108*n)};}
 void setNormalizedTimeForTesting(float value){time=std::clamp(value,0.f,.999999f)*CYCLE_SECONDS;}
private: float time=CYCLE_SECONDS*.27f;
};
