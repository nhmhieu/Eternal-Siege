#pragma once

#include <memory>
#include <vector>

#include "Projectiles.h"

class Entity;
class Monster;
class Ally;
class CombatManager;
class Map;
class Effects;
class AudioManager;

class GameContext {
public:
    float deltaTime = 0.f;

    // Các con trỏ quan sát, không sở hữu Entity.
    std::vector<Entity*> allEntity;
    std::vector<Entity*> players;
    std::vector<Entity*> enemies;
    std::vector<Monster*> monsters;
    std::vector<Ally*> allies;

    // GameContext sở hữu projectile.
    std::vector<std::unique_ptr<Projectiles>> projectiles;

    CombatManager* combatManager = nullptr;
    Effects* effects = nullptr;
    AudioManager* audioManager = nullptr;
    bool allySkillsEnabled = false;
    bool paused = false;

    // Map do GameplayState sở hữu, GameContext chỉ quan sát.
    const Map* map = nullptr;

    GameContext() = default;
    ~GameContext() = default;

    GameContext(const GameContext&) = delete;
    GameContext& operator=(const GameContext&) = delete;

    void removeFromTargetViews(const Entity* entity);
    void forgetEntity(const Entity* entity);
};
