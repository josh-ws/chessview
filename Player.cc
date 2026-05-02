#include "Player.h"
#include "Bitboard.h"
#include "Types.h"
#include <stdexcept>
#include <vector>

static int TileEval(const Position &p, std::function<int(int, int)> eval)
{
    auto score = 0;
    for (int col = 0; col < GRID_LENGTH; col++)
        for (int row = 0; row < GRID_LENGTH; row++)
            if (GetPiece(p, col, row) != NONE)
                score += eval(col, row);
    return score;
}

const static std::vector<Player> players = {
    Player{
        .name = "random",
        .description = "Plays completely random moves using mt19937",
        .evaluation = [](const Position &, const Move &) { return 0; },
    },
    Player{
        .name = "whitesquares",
        .description = "Plays moves that land pieces on white tiles",
        .evaluation = [](Position &p, const Move &) {
            return TileEval(p, [&](int col, int row) {
                return (col + row) % 1 == 0;
            });
        },
    },
    Player{
        .name = "blacksquares",
        .description = "Plays moves that land pieces on black tiles",
        .evaluation = [](Position &p, const Move &) {
            return TileEval(p, [&](int col, int row) {
                return (col + row) % 1 != 0;
            });
        },
    },
    Player{
        .name = "center",
        .description = "Moves pieces as close to the center as possible",
        .evaluation = [](Position &p, const Move &) {
            const static std::vector<int> bonus = {0, 1, 2, 3, 3, 2, 1, 0};
            return TileEval(p, [&](int col, int) {
                return bonus[col];
            });
        },
    },
    Player{
        .name = "min_oppt",
        .description = "Tries to minimize the number of moves the opponent has",
        .evaluation = [](Position &p, const Move &) {
            return -GenerateMoves(p).size();
        },
    },
    Player{
        .name = "max_oppt",
        .description = "Tries to maximize the number of moves the opponent has",
        .evaluation = [](Position &p, const Move &) {
            return GenerateMoves(p).size();
        },
    },
    Player{
        .name = "min_self",
        .description = "Tries to minimize the number of moves that the player has",
        .evaluation = [](Position &p, const Move &) {
            const auto moves = GenerateMoves(p);
            int score = 0;
            for (const auto &move : moves) {
                const auto u = MakeMove(p, move);
                score -= GenerateMoves(p).size();
                UndoMove(p, move, u);
            }
            return score;
        },
    },
    Player{
        .name = "max_self",
        .description = "Tries to maximize the number of moves that the player has",
        .evaluation = [](Position &p, const Move &) {
            const auto moves = GenerateMoves(p);
            int score = 0;
            for (const auto &move : moves) {
                const auto u = MakeMove(p, move);
                score += GenerateMoves(p).size();
                UndoMove(p, move, u);
            }
            return score;
        },
    },
    Player{
        .name = "defensive",
        .description = "Tries to minimize the number of own pieces under attack",
        .evaluation = [](Position &p, const Move &) {
            const auto me = p.whoseturn;
            int score = 0;
            for (int col = 0; col < GRID_LENGTH; col++)
                for (int row = 0; row < GRID_LENGTH; row++) {
                    const auto piece = GetPiece(p, col, row);
                    if (piece != NONE && ColorOf(piece) == (me ^ 1)) {
                        if (IsAttacked(p, col, row, me)) {
                            score--;
                        }
                    }
                }
            return score;
        },
    },
    Player{
        .name = "offensive",
        .description = "Tries to maximize the number of enemy pieces under attack",
        .evaluation = [](Position &p, const Move &) {
            const auto me = p.whoseturn;
            int score = 0;
            for (int col = 0; col < GRID_LENGTH; col++)
                for (int row = 0; row < GRID_LENGTH; row++) {
                    const auto piece = GetPiece(p, col, row);
                    if (piece != NONE && ColorOf(piece) == me) {
                        if (IsAttacked(p, col, row, me ^ 1)) {
                            score++;
                        }
                    }
                }
            return score;
        },
    },
    Player{
        .name = "reckless",
        .description = "Tries to maximize the number of own pieces under attack",
        .evaluation = [](Position &p, const Move &) {
            const auto me = p.whoseturn;
            int score = 0;
            for (int col = 0; col < GRID_LENGTH; col++)
                for (int row = 0; row < GRID_LENGTH; row++) {
                    const auto piece = GetPiece(p, col, row);
                    if (piece != NONE && ColorOf(piece) == (me ^ 1)) {
                        if (IsAttacked(p, col, row, me)) {
                            score++;
                        }
                    }
                }
            return score;
        },
    },
    Player{
        .name = "pacifist",
        .description = "Tries to minimize the number of enemy pieces under attack",
        .evaluation = [](Position &p, const Move &) {
            const auto me = p.whoseturn;
            int score = 0;
            for (int col = 0; col < GRID_LENGTH; col++)
                for (int row = 0; row < GRID_LENGTH; row++) {
                    const auto piece = GetPiece(p, col, row);
                    if (piece != NONE && ColorOf(piece) == me) {
                        if (IsAttacked(p, col, row, me ^ 1)) {
                            score--;
                        }
                    }
                }
            return score;
        },
    },
    Player{
        .name = "edge",
        .description = "Tries to move pieces towards the edge of the board",
        .evaluation = [](Position &p, const Move &) {
            const static std::vector<int> bonus = {3, 2, 1, 0, 0, 1, 2, 3};
            return TileEval(p, [&](int col, int) {
                return bonus[col];
            });
        },
    },
    Player{
        .name = "aggressive",
        .description = "Tries to move pieces towards the enemy's side",
        .evaluation = [](Position &p, const Move &) {
            const static std::vector<int> bonus = {0, 1, 2, 3, 4, 5, 6, 7};
            int sum = 0;
            for (int col = 0; col < GRID_LENGTH; col++)
                for (int row = 0; row < GRID_LENGTH; row++) {
                    const auto piece = GetPiece(p, col, row);
                    if (piece != NONE) {
                        if (ColorOf(piece) == CWHITE) {
                            sum += bonus[row];
                        }
                        else {
                            sum += -bonus[row];
                        }
                    }
                }
            return sum;
        },
    },
    Player{
        .name = "passive",
        .description = "Tries not to move pieces towards the enemy's side",
        .evaluation = [](Position &p, const Move &) {
            const static std::vector<int> bonus = {0, 1, 2, 3, 4, 5, 6, 7};
            int sum = 0;
            for (int col = 0; col < GRID_LENGTH; col++)
                for (int row = 0; row < GRID_LENGTH; row++) {
                    const auto piece = GetPiece(p, col, row);
                    if (piece != NONE) {
                        if (ColorOf(piece) == CWHITE) {
                            sum += -bonus[row];
                        }
                        else {
                            sum += bonus[row];
                        }
                    }
                }
            return sum;
        },
    }};

std::vector<Player> GetPlayerList() { return players; }

bool IsValidPlayer(const std::string &name)
{
    for (const auto &player : players)
        if (player.name == name)
            return true;
    return false;
}

Player MakePlayer(const std::string &name)
{
    for (const auto &player : players)
        if (player.name == name)
            return player;
    throw std::runtime_error("invalid player: " + name);
}
