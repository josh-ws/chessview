#include "Player.h"
#include <cstdlib>
#include <functional>
#include <random>
#include <stdexcept>
#include <vector>

using EvalFunction = std::function<int(const Board &, const Move &)>;

static int RandomEvalFunction(const Board &, const Move &) {
    return 0; // all moves are equally likely.
}

const static std::vector<Player> players = {
    Player{
        .name = "random",
        .description = "Plays completely random moves using mt19937",
        .evaluation = RandomEvalFunction,
    },
};

std::vector<Player> GetPlayerList() { return players; }

bool IsValidPlayer(const std::string &name) {
    for (const auto &player : players)
        if (player.name == name)
            return true;
    return false;
}

Player MakePlayer(const std::string &name) {
    for (const auto &player : players)
        if (player.name == name)
            return player;
    throw std::runtime_error("invalid player: " + name);
}
