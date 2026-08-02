#include "UpgradeManager.h"
#include "Ally.h"
#include "Player.h"

#include <algorithm>

int UpgradeManager::getLevel(UpgradeType type) const {
    switch (type) {
    case UpgradeType::Damage:   return damageLevel;
    case UpgradeType::Vitality: return vitalityLevel;
    case UpgradeType::FireRate: return fireRateLevel;
    }
    return 0;
}

int UpgradeManager::getCost(UpgradeType type) const {
    return 20 + getLevel(type) * 15;
}

UpgradePreview UpgradeManager::preview(UpgradeType type) const {
    const int level = getLevel(type);
    const int cost = getCost(type);
    UpgradePreview result{
        level, std::min(level + 1, 5), cost,
        gold >= cost && level < 5, level >= 5, "", "", ""};
    switch (type) {
    case UpgradeType::Damage:
        result.name = "TEAM DAMAGE";
        result.effectLine1 = "Player Damage +4";
        result.effectLine2 = "Ally Damage +3";
        break;
    case UpgradeType::Vitality:
        result.name = "TEAM VITALITY";
        result.effectLine1 = "Player Max HP +25";
        result.effectLine2 = "Ally Max HP +15";
        break;
    case UpgradeType::FireRate:
        result.name = "TEAM FIRE RATE";
        result.effectLine1 = "Player Cooldown x0.84";
        result.effectLine2 = "Ally Cooldown x0.88";
        break;
    }
    return result;
}

bool UpgradeManager::purchase(
    UpgradeType type, Player& player,
    std::vector<std::unique_ptr<Ally>>& allies) {
    const int cost = getCost(type);
    if (gold < cost || getLevel(type) >= 5) return false;
    gold -= cost;

    switch (type) {
    case UpgradeType::Damage:
        ++damageLevel;
        player.setAttackPower(player.getAttackPower() + 4.f);
        for (auto& ally : allies) {
            ally->setAttackPower(ally->getAttackPower() + 3.f);
        }
        break;

    case UpgradeType::Vitality:
        ++vitalityLevel;
        player.increaseMaxHealth(25.f);
        for (auto& ally : allies) ally->increaseMaxHealth(15.f);
        break;

    case UpgradeType::FireRate:
        ++fireRateLevel;
        player.setAttackCooldown(player.getAttackCooldown() * 0.84f);
        for (auto& ally : allies) {
            ally->setAttackCooldown(ally->getAttackCooldown() * 0.88f);
        }
        break;
    }

    purchaseHistory.push_back({type, cost});
    return true;
}

bool UpgradeManager::undoLastPurchase(
    Player& player,
    std::vector<std::unique_ptr<Ally>>& allies) {
    if (purchaseHistory.empty()) return false;

    const PurchaseRecord record = purchaseHistory.back();
    switch (record.type) {
    case UpgradeType::Damage:
        if (damageLevel <= 0) return false;
        --damageLevel;
        player.setAttackPower(
            std::max(0.f, player.getAttackPower() - 4.f));
        for (auto& ally : allies) {
            ally->setAttackPower(
                std::max(0.f, ally->getAttackPower() - 3.f));
        }
        break;

    case UpgradeType::Vitality:
        if (vitalityLevel <= 0) return false;
        --vitalityLevel;
        player.decreaseMaxHealth(25.f);
        for (auto& ally : allies) ally->decreaseMaxHealth(15.f);
        break;

    case UpgradeType::FireRate:
        if (fireRateLevel <= 0) return false;
        --fireRateLevel;
        player.setAttackCooldown(player.getAttackCooldown() / 0.84f);
        for (auto& ally : allies) {
            ally->setAttackCooldown(ally->getAttackCooldown() / 0.88f);
        }
        break;
    }

    gold += record.cost;
    purchaseHistory.pop_back();
    return true;
}
