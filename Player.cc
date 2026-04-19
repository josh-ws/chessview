#include "Player.h"
#include <random>
#include <stdexcept>
#include <vector>

template <typename T>
T getRandom(const std::vector<T> &vec) {
    static std::random_device device;
    static std::mt19937 mt{device()};
    std::uniform_int_distribution<> distrib(0, vec.size() - 1);
    return vec[distrib(mt)];
}

static Move RandomSelector(Board &board) {
    return getRandom(board.getMoves());
}

const static std::vector<Player> players = {
    Player{
        .name = "random",
        .description = "Plays completely random moves using mt19937",
        .moveSelector = RandomSelector,
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
