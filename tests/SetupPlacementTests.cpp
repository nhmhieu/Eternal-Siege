#include "AllyPlacementModel.h"

#include <array>
#include <cassert>
#include <iostream>

int main() {
    AllyPlacementModel model;

    // 1. A new session starts empty and cannot start.
    assert(model.deployedCount() == 0);
    assert(!model.isComplete());
    assert(!model.orderedPositions());
    assert(model.firstUnplaced() == AllyType::Damian);

    // 2. Every fixed Ally identity owns a separate slot.
    for (const AllyType type : AllyPlacementModel::ALLY_ORDER) {
        assert(!model.placement(type));
    }

    // 3-4. Arbitrary click order does not change identity.
    assert(model.place(AllyType::Lucas, {8, 8}) ==
           AllyPlacementModel::PlaceResult::Placed);
    assert(model.place(AllyType::Damian, {2, 3}) ==
           AllyPlacementModel::PlaceResult::Placed);
    assert(model.place(AllyType::Junior, {6, 5}) ==
           AllyPlacementModel::PlaceResult::Placed);
    assert(model.place(AllyType::Evangeline, {3, 7}) ==
           AllyPlacementModel::PlaceResult::Placed);
    assert(model.placement(AllyType::Lucas) == sf::Vector2i(8, 8));
    assert(model.placement(AllyType::Damian) == sf::Vector2i(2, 3));
    assert(model.placement(AllyType::Junior) == sf::Vector2i(6, 5));
    assert(model.placement(AllyType::Evangeline) == sf::Vector2i(3, 7));

    // 5. Two Allies cannot occupy one cell.
    assert(model.place(AllyType::Damian, {8, 8}) ==
           AllyPlacementModel::PlaceResult::Occupied);
    assert(model.placement(AllyType::Damian) == sf::Vector2i(2, 3));
    assert(model.placement(AllyType::Lucas) == sf::Vector2i(8, 8));

    // 6. Moving an Ally releases its old cell.
    assert(model.place(AllyType::Lucas, {9, 8}) ==
           AllyPlacementModel::PlaceResult::Moved);
    assert(!model.allyAt({8, 8}));
    assert(model.allyAt({9, 8}) == AllyType::Lucas);

    // 7. Remove returns the exact identity to the undeployed state.
    assert(model.removeAt({6, 5}) == AllyType::Junior);
    assert(!model.placement(AllyType::Junior));
    assert(model.deployedCount() == 3);
    assert(!model.isComplete());
    assert(model.firstUnplaced() == AllyType::Junior);
    assert(model.place(AllyType::Junior, {5, 6}) ==
           AllyPlacementModel::PlaceResult::Placed);

    // 9-10. Start readiness is true only with exactly all four slots.
    assert(model.isComplete());
    assert(model.deployedCount() == 4);

    // 11. Output order is always Damian, Evangeline, Junior, Lucas.
    const auto ordered = model.orderedPositions();
    assert(ordered);
    assert((*ordered)[0] == sf::Vector2i(2, 3));
    assert((*ordered)[1] == sf::Vector2i(3, 7));
    assert((*ordered)[2] == sf::Vector2i(5, 6));
    assert((*ordered)[3] == sf::Vector2i(9, 8));

    // 8 and 12. Reset/new-session semantics clear all four placements.
    model.reset();
    assert(model.deployedCount() == 0);
    assert(!model.isComplete());
    for (const AllyType type : AllyPlacementModel::ALLY_ORDER) {
        assert(!model.placement(type));
    }

    std::cout << "Setup placement tests passed\n";
    return 0;
}
