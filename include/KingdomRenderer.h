#pragma once
#include "TextureManager.h"
#include <SFML/Graphics.hpp>
#include <array>
#include <optional>
#include "KingdomRenderContracts.h"
class KingdomRenderer {
public:
 static constexpr std::size_t OCCLUDER_COUNT=10;
 static constexpr std::size_t BRIDGE_FAR_INDEX=4,BRIDGE_NEAR_INDEX=5;
 explicit KingdomRenderer(TextureManager&);
 void drawBase(sf::RenderTarget&)const;
 void drawWater(sf::RenderTarget&,float,float)const;
 void drawOccluder(sf::RenderTarget&,std::size_t)const;
 float occluderSortKey(std::size_t i)const{return i<OCCLUDER_COUNT?KINGDOM_OCCLUDER_KEYS[i]:0.f;}
private:
 std::optional<sf::Sprite> base;
 std::array<std::optional<sf::Sprite>,2> water;
 std::array<std::optional<sf::Sprite>,OCCLUDER_COUNT> occluders;
};
