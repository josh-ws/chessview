#ifndef PLAYER_H
#define PLAYER_H

#include "Bitboard.h"
#include <climits>
#include <functional>
#include <random>
#include <stdexcept>
#include <vector>

using EvalFunction = std::function<int(Position &, const Move &)>;

struct Player {
    std::string name;
    std::string description;
    EvalFunction evaluation;

    inline Move GetMove(Position &p) const
    {
        const auto moves = GenerateMoves(p);
        if (moves.size() == 0)
            throw std::runtime_error("empty move list passed to GetMove()");
        auto score = INT_MIN;
        auto ties = 1;
        auto selected = moves[0];
        static thread_local auto rng = std::mt19937{std::random_device{}()};
        for (const auto &move : moves) {
            const auto u = MakeMove(p, move);
            const auto newScore = evaluation(p, move);
            UndoMove(p, move, u);

            if (newScore > score) {
                score = newScore;
                ties = 1;
                selected = move;
            }
            else if (newScore == score) {
                ties += 1;
                if (std::uniform_int_distribution<int>(0, ties - 1)(rng) == 0)
                    selected = move;
            }
        }
        return selected;
    }
};

std::vector<Player> GetPlayerList();
bool IsValidPlayer(const std::string &name);
Player MakePlayer(const std::string &name);

#endif
