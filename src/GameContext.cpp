#include "GameContext.h"

#include "Ally.h"
#include "Entity.h"
#include "Monster.h"

#include <algorithm>

namespace {
template <typename Pointer>
void erasePointer(std::vector<Pointer>& pointers, const Entity* entity) {
    pointers.erase(
        std::remove_if(
            pointers.begin(),
            pointers.end(),
            [entity](Pointer pointer) {
                return static_cast<const Entity*>(pointer) == entity;
            }),
        pointers.end()
    );
}
}

void GameContext::removeFromTargetViews(const Entity* entity) {
    erasePointer(players, entity);
    erasePointer(enemies, entity);
    erasePointer(monsters, entity);
    erasePointer(allies, entity);
}

void GameContext::forgetEntity(const Entity* entity) {
    removeFromTargetViews(entity);
    erasePointer(allEntity, entity);
}
