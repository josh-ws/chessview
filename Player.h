#ifndef PLAYER_H
#define PLAYER_H

#include "Bitboard.h"
#include <functional>
#include <string>
#include <vector>

using EvalFunction = std::function<int(Position &, const Move &)>;

struct Player {
    std::string name;
    std::string description;
    EvalFunction evaluation;
    Move GetMove(Position &p) const;
};

std::vector<Player> GetPlayerList();
bool IsValidPlayer(const std::string &name);
Player MakePlayer(const std::string &name);

#endif
