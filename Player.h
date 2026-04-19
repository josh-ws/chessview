#ifndef PLAYER_H
#define PLAYER_H

#include "Board.h"
#include "Move.h"
#include <functional>
#include <vector>

struct Player {
    std::string name;
    std::string description;
    std::function<Move(Board &)> moveSelector;

    inline Move GetMove(Board &board) const { return moveSelector(board); }
};

std::vector<Player> GetPlayerList();
bool IsValidPlayer(const std::string &name);
Player MakePlayer(const std::string &name);

#endif
