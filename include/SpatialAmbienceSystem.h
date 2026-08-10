#pragma once
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <cmath>
class SpatialAmbienceSystem{
public:static float gain(sf::Vector2f listener,sf::Vector2f source,float inner,float outer){const float d=std::hypot(listener.x-source.x,listener.y-source.y);if(d<=inner)return 1;if(d>=outer)return 0;const float t=(d-inner)/(outer-inner);return 1-t*t*(3-2*t);}};
