#include "EntityLifecycle.h"

#include "CombatManager.h"
#include "Entity.h"
#include "GameContext.h"

namespace EntityLifecycle {

void invalidateReferencesTo(
    const std::vector<Entity*>& entities,
    GameContext& context,
    CombatManager& combatManager,
    bool removeFromAllViews
) {
    // allEntity is still a valid snapshot here: owners are erased only after
    // every observer and cache has forgotten all victims.
    for (Entity* victim : entities) {
        if (!victim) continue;

        for (Entity* observer : context.allEntity) {
            if (observer) {
                observer->forgetEntity(victim);
            }
        }
        combatManager.forgetEntity(victim);

        if (removeFromAllViews) {
            context.forgetEntity(victim);
        } else {
            context.removeFromTargetViews(victim);
        }
    }
}

}
