#include "UpgradeManager.h"
#include "Ally.h"
#include "Player.h"
#include "Weapon.h"

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

bool UpgradeManager::purchase(
    UpgradeType type, Player& player,
    std::vector<std::unique_ptr<Ally>>& allies) {
    const int cost = getCost(type);
    if (gold < cost || getLevel(type) >= 5) return false;
    gold -= cost;

    switch (type) {
    case UpgradeType::Damage:
        ++damageLevel;
        if (player.getCurrentWeapon()) {
            player.getCurrentWeapon()->setDamage(
                player.getCurrentWeapon()->getDamage() + 4);
        }
        for (auto& ally : allies) {
            if (ally->getCurrentWeapon()) {
                ally->getCurrentWeapon()->setDamage(
                    ally->getCurrentWeapon()->getDamage() + 3);
            }
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
        if (player.getCurrentWeapon()) {
            player.getCurrentWeapon()->setDamage(
                std::max(0, player.getCurrentWeapon()->getDamage() - 4));
        }
        for (auto& ally : allies) {
            if (ally->getCurrentWeapon()) {
                ally->getCurrentWeapon()->setDamage(
                    std::max(0, ally->getCurrentWeapon()->getDamage() - 3));
            }
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
