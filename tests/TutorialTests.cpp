#include "Map.h"
#include "TutorialOverlay.h"
#include "WaveManager.h"

#include <cassert>
#include <random>

namespace {
std::unique_ptr<Monster> gatedWaveUpdate(
    TutorialController& tutorial, WaveManager& waves, const Map& map,
    float deltaTime, bool noMonstersAlive = true) {
    if (tutorial.blocksGameplayUpdate()) return nullptr;
    return waves.update(deltaTime, map, noMonstersAlive);
}
}

int main() {
    TutorialController tutorial;
    tutorial.resetForNewGame();
    assert(tutorial.getMode() == TutorialMode::FirstLaunch);
    assert(tutorial.blocksGameplayUpdate());
    assert(tutorial.blocksGameplayInput());

    Map map(15, 15);
    WaveManager waves;
    for (int frame = 0; frame < 20; ++frame) {
        assert(!gatedWaveUpdate(tutorial, waves, map, 0.1f));
    }
    assert(waves.getCurrentWave() == 1);

    assert(tutorial.handleKeyPressed(TutorialKey::Help) ==
           TutorialTransition::Consumed);
    assert(tutorial.getMode() == TutorialMode::FirstLaunch);
    assert(tutorial.handleKeyPressed(TutorialKey::Help) ==
           TutorialTransition::Consumed);
    assert(tutorial.handleKeyReleased(TutorialKey::Help));

    assert(tutorial.handleKeyPressed(TutorialKey::Enter) ==
           TutorialTransition::InitialClosed);
    assert(tutorial.getMode() == TutorialMode::Hidden);
    assert(tutorial.blocksGameplayUpdate());
    assert(tutorial.blocksGameplayInput());
    assert(tutorial.handleKeyPressed(TutorialKey::Enter) ==
           TutorialTransition::Consumed);
    assert(tutorial.handleKeyPressed(TutorialKey::Help) ==
           TutorialTransition::Consumed);
    assert(!gatedWaveUpdate(tutorial, waves, map, 1.f));
    assert(tutorial.handleKeyReleased(TutorialKey::Enter));
    tutorial.finishInputRelease();
    assert(!tutorial.blocksGameplayUpdate());

    assert(!gatedWaveUpdate(tutorial, waves, map, 0.40f));
    assert(tutorial.handleKeyPressed(TutorialKey::Help) ==
           TutorialTransition::Reopened);
    assert(tutorial.blocksGameplayUpdate());
    assert(tutorial.handleKeyReleased(TutorialKey::Help));
    for (int frame = 0; frame < 50; ++frame) {
        assert(!gatedWaveUpdate(tutorial, waves, map, 0.1f));
    }
    assert(waves.getCurrentWave() == 1);

    assert(tutorial.handleKeyPressed(TutorialKey::Help) ==
           TutorialTransition::Closed);
    assert(tutorial.handleKeyPressed(TutorialKey::Help) ==
           TutorialTransition::Consumed);
    assert(tutorial.handleKeyReleased(TutorialKey::Help));
    tutorial.finishInputRelease();
    assert(!gatedWaveUpdate(tutorial, waves, map, 0.39f));
    assert(gatedWaveUpdate(tutorial, waves, map, 0.02f) != nullptr);

    const int waveBeforeReopen = waves.getCurrentWave();
    tutorial.handleKeyPressed(TutorialKey::Help);
    tutorial.handleKeyReleased(TutorialKey::Help);
    assert(!gatedWaveUpdate(tutorial, waves, map, 10.f, false));
    assert(waves.getCurrentWave() == waveBeforeReopen);
    tutorial.handleKeyPressed(TutorialKey::Enter);
    tutorial.handleKeyReleased(TutorialKey::Enter);
    tutorial.finishInputRelease();
    assert(waves.getCurrentWave() == waveBeforeReopen);

    std::mt19937 observed(0xE71Eu);
    std::mt19937 control(0xE71Eu);
    tutorial.resetForNewGame();
    tutorial.handleKeyPressed(TutorialKey::Enter);
    tutorial.handleKeyReleased(TutorialKey::Enter);
    tutorial.finishInputRelease();
    assert(observed() == control());

    Map sameMap(15, 15);
    for (int row = 0; row < map.getHeight(); ++row) {
        for (int column = 0; column < map.getWidth(); ++column) {
            assert(map.getTileType(column, row) ==
                   sameMap.getTileType(column, row));
        }
    }

    tutorial.resetForNewGame();
    assert(tutorial.getMode() == TutorialMode::FirstLaunch);
    assert(tutorial.blocksGameplayUpdate());
    return 0;
}
