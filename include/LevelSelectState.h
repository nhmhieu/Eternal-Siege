#pragma once
#include "State.h"
#include "StateMachine.h"
#include "TextureManager.h"
#include "AudioManager.h"
#include "LevelDefinition.h"
#include "GameProgress.h"

class LevelSelectState : public State {
public:
    LevelSelectState(StateMachine&, sf::RenderWindow&, TextureManager&, AudioManager&, GameProgress&);
    void onEnter() override; void onExit() override;
    void handleEvent(const sf::Event&) override; void update(float) override;
    void render(sf::RenderWindow&) override;
private:
    StateMachine& machine; sf::RenderWindow& window; TextureManager& textures; AudioManager& audio;
    GameProgress& progress;
    sf::Font font; bool fontLoaded=false; std::size_t selected=0; bool transition=false;
    void confirm();
};
