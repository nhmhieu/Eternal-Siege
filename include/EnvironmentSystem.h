#pragma once
#include <algorithm>
enum class KingdomWeather{Clear,LightRain,Mist};
class EnvironmentSystem{
public:
 void update(float dt){elapsed+=std::max(0.f,dt);}
 void advanceWeather(){weather=static_cast<KingdomWeather>((static_cast<int>(weather)+1)%3);}
 KingdomWeather getWeather()const{return weather;}
 float time()const{return elapsed;}
private:KingdomWeather weather=KingdomWeather::Clear;float elapsed=0;
};
