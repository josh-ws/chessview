#include "Player.h"
#include "Bitboard.h"
#include <algorithm>
#include <bit>
#include <climits>
#include <random>
#include <raylib.h>
#include <stdexcept>
#include <vector>

Move Player::GetMove(Position &p) const
{
    const auto moves = GenerateMoves(p);
    if (moves.size() == 0)
        throw std::runtime_error("empty move list passed to GetMove()");
    auto score = INT_MIN;
    auto ties = 1;
    auto selected = moves[0];
    static thread_local auto rng = std::mt19937{std::random_device{}()};
    for (const auto &move : moves) {
        const auto newScore = evaluation(p, move);
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

static int Chebyshev(int a, int b)
{
    int dx = std::abs((a & 7) - (b & 7));
    int dy = std::abs((a >> 3) - (b >> 3));
    return std::max(dx, dy);
}

static int EvaluateMaterial(const Position &p, CColor color)
{
    static const int pieceScores[] = {1, 0, 8, 3, 3, 5, 0};
    auto score = 0;
    for (const auto piece : {QUEEN, ROOK, BISHOP, KNIGHT, ROOK, PAWN}) {
        const auto count = std::popcount(p.bitboards[color][piece]);
        score += (count * pieceScores[piece]);
    }
    return score;
};

static int EvaluationCCCP(Position &p, const Move &m)
{
    const auto u = MakeMove(p, m);
    const auto state = GetPositionState(p);
    const auto isCheck = IsCheck(p, p.whoseturn);
    UndoMove(p, m, u);

    if (isCheck) {
        if (state == S_CHECKMATE) {
            return 1'000'000;
        }
        else {
            return 100'000;
        }
    }

    const auto isCapture = p.occupancy[p.whoseturn ^ 1] & (1ULL << m.to);
    if (isCapture) {
        return 1'000;
    }

    const auto fromRow = (m.from / 8);
    const auto toRow = (m.to / 8);
    const auto rowDelta = p.whoseturn == CWHITE ? toRow - fromRow : fromRow - toRow;
    return rowDelta;
}

static int EvaluationRawMaterial(Position &p, const Move &m)
{
    const auto u = MakeMove(p, m);
    const auto score = EvaluateMaterial(p, static_cast<CColor>(p.whoseturn ^ 1)) - EvaluateMaterial(p, p.whoseturn);
    UndoMove(p, m, u);
    return score;
}

static int EvaluationHuddle(Position &p, const Move &m)
{
    const auto color = p.whoseturn;
    const auto u = MakeMove(p, m);
    const auto kingSq = std::countr_zero(p.bitboards[color][KING]);
    auto total = 0;
    for (int sq = 0; sq < 64; sq++) {
        if (sq == kingSq)
            continue;
        const auto pc = GetPiece(p, ColOf(sq), RowOf(sq));
        if (pc == NONE || ColorOf(pc) != color)
            continue;
        total += Chebyshev(sq, kingSq);
    }
    UndoMove(p, m, u);
    return total;
}

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
