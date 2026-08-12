#pragma once

#include <vector>

class Ally;
class Map;
class Monster;

namespace EntityCollision {

void separateLivingEntities(
    const Map& map,
    const std::vector<Monster*>& monsters,
    const std::vector<Ally*>& allies,
    float dt = 0.016f
);

}
