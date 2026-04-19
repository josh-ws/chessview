#include "Player.h"
#include "Types.h"
#include <cstdlib>
#include <functional>
#include <random>
#include <stdexcept>
#include <vector>

using EvalFunction = std::function<int(const Board &, const Move &)>;

static int RandomEval(const Board &, const Move &) {
    return 0; // all moves are equally likely.
}

static int TileEval(const Board &board, std::function<int(int, int)> eval) {
    auto score = 0;
    for (int col = 0; col < GRID_LENGTH; col++)
        for (int row = 0; row < GRID_LENGTH; row++)
            if (board.pieceAt(col, row) != EMPTY)
                score += eval(col, row);
    return score;
}

static int WhiteSquaresEval(const Board &board, const Move &) {
    return TileEval(board, [&](int col, int row) {
        return board.colorAt(col, row) == PIECE_WHITE;
    });
}

static int BlackSquaresEval(const Board &board, const Move &) {
    return TileEval(board, [&](int col, int row) {
        return board.colorAt(col, row) == PIECE_BLACK;
    });
}

static int CenterEval(const Board &board, const Move &) {
    const static std::vector<int> bonus = {0, 1, 2, 3, 3, 2, 1, 0};
    return TileEval(board, [&](int col, int row) {
        return bonus[col];
    });
}

const static std::vector<Player> players = {
    Player{
        .name = "random",
        .description = "Plays completely random moves using mt19937",
        .evaluation = RandomEval,
    },
    Player{
        .name = "whitesquares",
        .description = "Plays moves that land pieces on white tiles",
        .evaluation = WhiteSquaresEval,
    },
    Player{
        .name = "blacksquares",
        .description = "Plays moves that land pieces on black tiles",
        .evaluation = BlackSquaresEval,
    },
    Player{
        .name = "center",
        .description = "Moves pieces as close to the center as possible",
        .evaluation = CenterEval,
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
