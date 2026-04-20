#ifndef PLAYER_H
#define PLAYER_H

#include "Board.h"
#include "Move.h"
#include <climits>
#include <functional>
#include <random>
#include <stdexcept>
#include <vector>

using EvalFunction = std::function<int(Board &, const Move &)>;

struct Player {
    std::string name;
    std::string description;
    EvalFunction evaluation;

    inline Move GetMove(Board &board) const {
        const auto moves = board.getMoves();
        if (moves.empty())
            throw std::runtime_error("empty move list passed to GetMove()");
        auto score = INT_MIN;
        auto idx = size_t(0);
        auto ties = 1;
        static thread_local auto rng = std::mt19937{std::random_device{}()};
        for (size_t i = 0; i < moves.size(); ++i) {
            const auto &move = moves[i];
            const auto u = board.MakeNewMove(move);
            const auto newScore = evaluation(board, move);
            board.UndoMove(u);
            if (newScore > score) {
                score = newScore;
                idx = i;
                ties = 1;
            } else if (newScore == score) {
                ties += 1;
                if (std::uniform_int_distribution<int>(0, ties - 1)(rng) == 0)
                    idx = i;
            }
        }
        return moves[idx];
    }
};

std::vector<Player> GetPlayerList();
bool IsValidPlayer(const std::string &name);
Player MakePlayer(const std::string &name);

#endif
