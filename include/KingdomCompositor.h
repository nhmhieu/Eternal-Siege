#pragma once
#include "EnvironmentSystem.h"
#include <SFML/Graphics.hpp>
#include <memory>
class KingdomCompositor{public:bool ensure(sf::Vector2u);sf::RenderTexture&begin(const sf::View&,sf::Color={8,15,18});void applyAmbient(sf::Color);void finish();void drawWorld(sf::RenderTarget&)const;void drawWeather(sf::RenderTarget&,KingdomWeather,float)const;std::size_t initializationCount()const{return initializations;}std::size_t recreationCount()const{return recreations;}sf::Vector2u size()const{return currentSize;}private:std::unique_ptr<sf::RenderTexture>target;sf::Vector2u currentSize{};std::size_t initializations=0,recreations=0;};
