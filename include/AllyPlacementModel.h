#pragma once

#include "AllyConfig.h"

#include <SFML/System/Vector2.hpp>

#include <array>
#include <cstddef>
#include <optional>

class AllyPlacementModel {
public:
    static constexpr std::size_t ALLY_COUNT = 4;
    static constexpr std::array<AllyType, ALLY_COUNT> ALLY_ORDER{{
        AllyType::Damian,
        AllyType::Evangeline,
        AllyType::Junior,
        AllyType::Lucas
    }};

    enum class PlaceResult {
        Placed,
        Moved,
        AlreadyThere,
        Occupied
    };

    PlaceResult place(AllyType type, sf::Vector2i cell) {
        const auto occupant = allyAt(cell);
        if (occupant && *occupant != type) {
            return PlaceResult::Occupied;
        }

        auto& slot = placements[indexOf(type)];
        if (slot && *slot == cell) {
            return PlaceResult::AlreadyThere;
        }
        const bool wasPlaced = slot.has_value();
        slot = cell;
        return wasPlaced ? PlaceResult::Moved : PlaceResult::Placed;
    }

    bool remove(AllyType type) {
        auto& slot = placements[indexOf(type)];
        if (!slot) return false;
        slot.reset();
        return true;
    }

    std::optional<AllyType> removeAt(sf::Vector2i cell) {
        const auto occupant = allyAt(cell);
        if (!occupant) return std::nullopt;
        placements[indexOf(*occupant)].reset();
        return occupant;
    }

    void reset() noexcept {
        for (auto& placement : placements) placement.reset();
    }

    const std::optional<sf::Vector2i>& placement(AllyType type) const {
        return placements[indexOf(type)];
    }

    std::optional<AllyType> allyAt(sf::Vector2i cell) const {
        for (std::size_t index = 0; index < placements.size(); ++index) {
            if (placements[index] && *placements[index] == cell) {
                return ALLY_ORDER[index];
            }
        }
        return std::nullopt;
    }

    std::optional<AllyType> firstUnplaced() const {
        for (std::size_t index = 0; index < placements.size(); ++index) {
            if (!placements[index]) return ALLY_ORDER[index];
        }
        return std::nullopt;
    }

    std::size_t deployedCount() const noexcept {
        std::size_t count = 0;
        for (const auto& placement : placements) {
            if (placement) ++count;
        }
        return count;
    }

    bool isComplete() const noexcept {
        return deployedCount() == ALLY_COUNT;
    }

    std::optional<std::array<sf::Vector2i, ALLY_COUNT>>
    orderedPositions() const {
        if (!isComplete()) return std::nullopt;
        std::array<sf::Vector2i, ALLY_COUNT> result{};
        for (std::size_t index = 0; index < placements.size(); ++index) {
            result[index] = *placements[index];
        }
        return result;
    }

private:
    static constexpr std::size_t indexOf(AllyType type) noexcept {
        return static_cast<std::size_t>(type);
    }

    std::array<std::optional<sf::Vector2i>, ALLY_COUNT> placements{};
};
