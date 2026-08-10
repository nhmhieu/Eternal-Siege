#pragma once

enum class Team {
    Player,
    Enemy,
    Neutral
};

// Hàm kiểm tra phe phái đặt chung luôn ở đây
inline bool isEnemy(Team team1, Team team2) {
    return team1 != team2;
}