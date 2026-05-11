#include "Player.h"
#include "../Bitboard.h"
#include "Eval.h"
#include <climits>
#include <random>
#include <raylib.h>
#include <stdexcept>
#include <vector>

const static std::vector<Player> players = {
    Player{
        .name = "random",
        .description = "Plays completely random moves using mt19937",
        .evaluation = [](const Position &, const Move &) { return 0; },
    },
    Player{
        .name = "cccp",
        .description = "Checkmate, check, capture, push",
        .evaluation = EvaluationCCCP,
    },
    Player{
        .name = "material",
        .description = "Greedily maximizes material",
        .evaluation = EvaluationRawMaterial,
    },
    Player{
        .name = "huddle",
        .description = "Huddles pieces around its own King",
        .evaluation = [](Position &p, const Move &m) {
            return EvaluationHuddle(p, m);
        },
    },
    Player{
        .name = "anti-huddle",
        .description = "Moves pieces away from its own King",
        .evaluation = [](Position &p, const Move &m) {
            return -EvaluationHuddle(p, m);
        },
    },
    Player{
        .name = "swarm",
        .description = "Swarms the enemy King with pieces",
        .evaluation = [](Position &p, const Move &m) {
            return EvaluationSwarm(p, m);
        },
    },
    Player{
        .name = "passive",
        .description = "Opposite of `swarm`, avoids swarming the enemy King",
        .evaluation = [](Position &p, const Move &m) {
            return -EvaluationSwarm(p, m);
        },
    },
    Player{
        .name = "glue",
        .description = "Keeps pieces as close together as possible",
        .evaluation = [](Position &p, const Move &m) {
            return EvaluationGlue(p, m);
        },
    },
    Player{
        .name = "repel",
        .description = "Pieces repel away from each other, opposite of `glue`",
        .evaluation = [](Position &p, const Move &m) {
            return -EvaluationGlue(p, m);
        },
    },
    Player{
        .name = "white-squares",
        .description = "Lands pieces on white squares",
        .evaluation = [](Position &p, const Move &m) {
            return EvaluationWhiteSquares(p, m);
        },
    },
    Player{
        .name = "black-squares",
        .description = "Lands pieces on black squares",
        .evaluation = [](Position &p, const Move &m) {
            return -EvaluationWhiteSquares(p, m);
        },
    }};

std::vector<Player> GetPlayerList()
{
    return players;
}

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

Move GetMove(const Player &player, Position &p)
{
    const auto moves = GenerateMoves(p);
    if (moves.size() == 0)
        throw std::runtime_error("empty move list passed to GetMove()");
    auto score = INT_MIN;
    auto ties = 1;
    auto selected = moves[0];
    static thread_local auto rng = std::mt19937{std::random_device{}()};
    for (const auto &move : moves) {
        const auto newScore = player.evaluation(p, move);
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
