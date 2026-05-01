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
    // Player{
    //     .name = "whitesquares",
    //     .description = "Plays moves that land pieces on white tiles",
    //     .evaluation = [](Position &p, const Move &) {
    //         return TileEval(p, [&](int col, int row) {
    //             return board.colorAt(col, row) == PIECE_WHITE;
    //         });
    //     },
    // },
    // Player{
    //     .name = "blacksquares",
    //     .description = "Plays moves that land pieces on black tiles",
    //     .evaluation = [](const Position &p, const Move &) {
    //         return TileEval(board, [&](int col, int row) {
    //             return board.colorAt(col, row) == PIECE_BLACK;
    //         });
    //     },
    // },
    // Player{
    //     .name = "center",
    //     .description = "Moves pieces as close to the center as possible",
    //     .evaluation = [](const Position &p, const Move &) {
    //         const static std::vector<int> bonus = {0, 1, 2, 3, 3, 2, 1, 0};
    //         return TileEval(board, [&](int col, int) {
    //             return bonus[col];
    //         });
    //     },
    // },
    // Player{
    //     .name = "min_oppt",
    //     .description = "Tries to minimize the number of moves the opponent has",
    //     .evaluation = [](const Position &p, const Move &) { return -board.getMoves().size(); },
    // },
    // Player{
    //     .name = "max_oppt",
    //     .description = "Tries to maximize the number of moves the opponent has",
    //     .evaluation = [](const Position &p, const Move &) { return board.getMoves().size(); },
    // },
    // Player{
    //     .name = "min_self",
    //     .description = "Tries to minimize the number of moves that the player has",
    //     .evaluation = [](const Position &p, const Move &) {
    //         int score = 0;
    //         for (const auto &move : board.getMoves()) {
    //             const auto u = board.MakeNewMove(move);
    //             score += -board.getMoves().size();
    //             board.UndoMove(u);
    //         }
    //         return score;
    //     },
    // },
    // Player{
    //     .name = "max_self",
    //     .description = "Tries to maximize the number of moves that the player has",
    //     .evaluation = [](const Position &p, const Move &) {
    //         int score = 0;
    //         for (const auto &move : board.getMoves()) {
    //             const auto u = board.MakeNewMove(move);
    //             score += board.getMoves().size();
    //             board.UndoMove(u);
    //         }
    //         return score;
    //     },
    // },
    // Player{
    //     .name = "defensive",
    //     .description = "Tries to minimize the number of own pieces under attack",
    //     .evaluation = [](Board &board, const Move &) {
    //         const auto me = board.WhoseTurn() ^ COLOR_MASK;
    //         int score = 0;
    //         for (int col = 0; col < GRID_LENGTH; col++)
    //             for (int row = 0; row < GRID_LENGTH; row++) {
    //                 const auto piece = board.pieceAt(col, row);
    //                 if (piece != EMPTY && (piece & COLOR_MASK) == me) {
    //                     if (board.isAttacked(col, row)) {
    //                         score--;
    //                     }
    //                 }
    //             }
    //         return score;
    //     },
    // },
    // Player{
    //     .name = "offensive",
    //     .description = "Tries to maximize the number of enemy pieces under attack",
    //     .evaluation = [](Board &board, const Move &) {
    //         const auto me = board.WhoseTurn() ^ COLOR_MASK;
    //         int score = 0;
    //         for (int col = 0; col < GRID_LENGTH; col++)
    //             for (int row = 0; row < GRID_LENGTH; row++) {
    //                 const auto piece = board.pieceAt(col, row);
    //                 if (piece != EMPTY && (piece & COLOR_MASK) != me) {
    //                     if (board.isAttacked(col, row)) {
    //                         score++;
    //                     }
    //                 }
    //             }
    //         return score;
    //     },
    // },
    // Player{
    //     .name = "reckless",
    //     .description = "Tries to maximize the number of own pieces under attack",
    //     .evaluation = [](Board &board, const Move &) {
    //         const auto me = board.WhoseTurn() ^ COLOR_MASK;
    //         int score = 0;
    //         for (int col = 0; col < GRID_LENGTH; col++)
    //             for (int row = 0; row < GRID_LENGTH; row++) {
    //                 const auto piece = board.pieceAt(col, row);
    //                 if (piece != EMPTY && (piece & COLOR_MASK) == me) {
    //                     if (board.isAttacked(col, row)) {
    //                         score++;
    //                     }
    //                 }
    //             }
    //         return score;
    //     },
    // },
    // Player{
    //     .name = "pacifist",
    //     .description = "Tries to minumize the number of enemy pieces under attack",
    //     .evaluation = [](Board &board, const Move &) {
    //         const auto me = board.WhoseTurn() ^ COLOR_MASK;
    //         int score = 0;
    //         for (int col = 0; col < GRID_LENGTH; col++)
    //             for (int row = 0; row < GRID_LENGTH; row++) {
    //                 const auto piece = board.pieceAt(col, row);
    //                 if (piece != EMPTY && (piece & COLOR_MASK) != me) {
    //                     if (board.isAttacked(col, row)) {
    //                         score--;
    //                     }
    //                 }
    //             }
    //         return score;
    //     },
    // },
    // Player{
    //     .name = "edge",
    //     .description = "Tries to move pieces towards the edge of the board",
    //     .evaluation = [](Board &board, const Move &) {
    //         const static std::vector<int> bonus = {3, 2, 1, 0, 0, 1, 2, 3};
    //         return TileEval(board, [&](int col, int) {
    //             return bonus[col];
    //         });
    //     },
    // },
    // Player{
    //     .name = "aggressive",
    //     .description = "Tries to move pieces towards the enemy's side",
    //     .evaluation = [](Board &board, const Move &move) {
    //         const static std::vector<int> bonus = {0, 1, 2, 3, 4, 5, 6, 7};
    //         int sum = 0;
    //         for (int col = 0; col < GRID_LENGTH; col++)
    //             for (int row = 0; row < GRID_LENGTH; row++) {
    //                 const auto piece = board.pieceAt(col, row);
    //                 if (piece != EMPTY) {
    //                     if ((piece & COLOR_MASK) == PIECE_WHITE) {
    //                         sum += bonus[row];
    //                     }
    //                     else {
    //                         sum += -bonus[row];
    //                     }
    //                 }
    //             }
    //         return sum;
    //     },
    // },
    // Player{
    //     .name = "passive",
    //     .description = "Tries not to move pieces towards the enemy's side",
    //     .evaluation = [](Board &board, const Move &) {
    //         const static std::vector<int> bonus = {0, 1, 2, 3, 4, 5, 6, 7};
    //         int sum = 0;
    //         for (int col = 0; col < GRID_LENGTH; col++)
    //             for (int row = 0; row < GRID_LENGTH; row++) {
    //                 const auto piece = board.pieceAt(col, row);
    //                 if (piece != EMPTY) {
    //                     if ((piece & COLOR_MASK) == PIECE_WHITE) {
    //                         sum += -bonus[row];
    //                     }
    //                     else {
    //                         sum += bonus[row];
    //                     }
    //                 }
    //             }
    //         return sum;
    //     },
};

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
