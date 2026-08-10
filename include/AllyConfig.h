#pragma once

#include <string_view>

enum class AllyType {
    Damian,
    Evangeline,
    Junior,
    Lucas
};

enum class WeaponType {
    Sword,
    Bow,
    Wand
};

struct AllyStats {
    float maxHealth;
    int attackDamage;
    float attackCooldown;
    float attackRange;
    WeaponType weaponType;
    float damageTakenMultiplier;
};

enum class AllySkillType {
    ExplosiveArrow,
    ShieldWall,
    ArcaneBurst,
    Whirlwind
};

struct AllySkillConfig {
    AllySkillType type;
    const char* name;
    float cooldown;
    float triggerRange;
    float effectDuration;
    float primaryMultiplier;
    float secondaryMultiplier;
    float secondaryRadius;
    int minimumEnemies;
};

const AllyStats& getAllyStats(AllyType type);
const AllySkillConfig& getAllySkillConfig(AllyType type);
const char* getAllyName(AllyType type);
const char* getAllyRole(AllyType type);
const char* getWeaponName(WeaponType type);
AllyType getAllyType(std::string_view name);
