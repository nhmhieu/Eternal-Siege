#pragma once

#include <vector>

class CombatManager;
class Entity;
class GameContext;

namespace EntityLifecycle {

void invalidateReferencesTo(
    const std::vector<Entity*>& entities,
    GameContext& context,
    CombatManager& combatManager,
    bool removeFromAllViews
);

}
