#pragma once

#include <cstddef>
#include <memory>
#include <vector>

class Ally;
class Player;

enum class UpgradeType {
    Damage,
    Vitality,
    FireRate
};

struct UpgradePreview {
    int level = 0;
    int nextLevel = 0;
    int cost = 0;
    bool affordable = false;
    bool maxLevel = false;
    const char* name = "";
    const char* effectLine1 = "";
    const char* effectLine2 = "";
};

class UpgradeManager {
public:
    void addGold(int amount) { gold += amount; }
    int getGold() const { return gold; }
    int getLevel(UpgradeType type) const;
    int getCost(UpgradeType type) const;
    UpgradePreview preview(UpgradeType type) const;

    bool purchase(UpgradeType type, Player& player,
                  std::vector<std::unique_ptr<Ally>>& allies);
    bool undoLastPurchase(Player& player,
                          std::vector<std::unique_ptr<Ally>>& allies);
    bool canUndo() const { return !purchaseHistory.empty(); }
    std::size_t getPendingPurchaseCount() const {
        return purchaseHistory.size();
    }
    void clearUndoHistory() { purchaseHistory.clear(); }

private:
    struct PurchaseRecord {
        UpgradeType type;
        int cost;
    };

    int gold = 25;
    int damageLevel = 0;
    int vitalityLevel = 0;
    int fireRateLevel = 0;
    std::vector<PurchaseRecord> purchaseHistory;
};
