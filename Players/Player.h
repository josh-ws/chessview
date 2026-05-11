#pragma once

#include <functional>
#include <string>

#include "../Bitboard.h"

using EvalFunction = std::function<int(Position &, const Move &)>;

struct Player {
    std::string name;
    std::string description;
    EvalFunction evaluation;
};

std::vector<Player> GetPlayerList();
bool IsValidPlayer(const std::string &name);
Player MakePlayer(const std::string &name);

Move GetMove(const Player &player, Position &p);
