#pragma once
#include "State.h"
#include "StateMachine.h"
#include "KingdomMap.h"
#include "TextureManager.h"
#include "AudioManager.h"
#include "Player.h"
#include "GameProgress.h"
#include "KingdomGateController.h"
#include "KingdomNpc.h"
#include "DayNightSystem.h"
#include "KingdomRenderer.h"
#include "EnvironmentSystem.h"
#include "NpcScheduleSystem.h"
#include "KingdomTransitionLatch.h"
#include "KingdomRouteCatalog.h"
#include "KingdomCompositor.h"
#include <memory>
#include <optional>

class KingdomState : public State {
public:
    KingdomState(StateMachine&, sf::RenderWindow&, TextureManager&, AudioManager&,
                 GameProgress&, bool returningFromDungeon=false);
    void onEnter() override; void onExit() override;
    void handleEvent(const sf::Event&) override;
    void update(float) override; void render(sf::RenderWindow&) override;
private:
    StateMachine& machine; sf::RenderWindow& window; TextureManager& textures; AudioManager& audio;
    GameProgress& progress; KingdomGateController gate;
    KingdomMap map; sf::Vector2f position; sf::Vector2f facing{0.f,-1.f};
    std::unique_ptr<Player> avatar;
    std::vector<std::unique_ptr<KingdomNpc>> npcs;
    std::unique_ptr<KingdomRenderer> renderer;
    sf::View worldView;
    sf::View uiView;
    sf::Vector2f cameraCenter;
    sf::Font font; bool fontLoaded=false; bool returning=false; bool transitioning=false;
    float elapsed=0.f, fade=1.f, promptAlpha=0.f;
    bool debugCollision=false;
    bool debugPerformance=false;
    float lastFrameDt=0.f;
    DayNightSystem dayNight;
    EnvironmentSystem environment;
    KingdomTransitionLatch transitionLatch;
    std::unique_ptr<KingdomCompositor> compositor;
    sf::RenderTexture* sceneTarget=nullptr;
    void drawText(sf::RenderWindow&, const std::string&, sf::Vector2f, unsigned, sf::Color) const;
};
