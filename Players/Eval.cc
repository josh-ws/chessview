#include <algorithm>
#include <bit>
#include <cstdlib>

#include "../Bitboard.h"

static int Chebyshev(int a, int b)
{
    int dx = std::abs((a & 7) - (b & 7));
    int dy = std::abs((a >> 3) - (b >> 3));
    return std::max(dx, dy);
}

static int DistanceSumFromKing(const Position &p, CColor color, int target)
{
    auto total = 0;
    for (int sq = 0; sq < 64; sq++) {
        const auto pc = GetPiece(p, ColOf(sq), RowOf(sq));
        if (pc == NONE || ColorOf(pc) != color)
            continue;
        total += Chebyshev(sq, target);
    }
    return total;
}

int EvaluateMaterial(const Position &p, CColor color)
{
    static const int pieceScores[] = {1, 0, 8, 3, 3, 5, 0};
    auto score = 0;
    for (const auto piece : {QUEEN, ROOK, BISHOP, KNIGHT, ROOK, PAWN}) {
        const auto count = std::popcount(p.bitboards[color][piece]);
        score += (count * pieceScores[piece]);
    }
    return score;
};

int EvaluationCCCP(Position &p, const Move &m)
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

int EvaluationRawMaterial(Position &p, const Move &m)
{
    const auto u = MakeMove(p, m);
    const auto score = EvaluateMaterial(p, static_cast<CColor>(p.whoseturn ^ 1)) - EvaluateMaterial(p, p.whoseturn);
    UndoMove(p, m, u);
    return score;
}

int EvaluationHuddle(Position &p, const Move &m)
{
    const auto color = p.whoseturn;
    const auto u = MakeMove(p, m);
    const auto kingSq = std::countr_zero(p.bitboards[color][KING]);
    const auto total = -DistanceSumFromKing(p, color, kingSq);
    UndoMove(p, m, u);
    return total;
}

int EvaluationSwarm(Position &p, const Move &m)
{
    const auto us = p.whoseturn;
    const auto them = static_cast<CColor>(p.whoseturn ^ 1);
    const auto u = MakeMove(p, m);
    const auto enemyKingSq = std::countr_zero(p.bitboards[them][KING]);
    const auto total = -DistanceSumFromKing(p, us, enemyKingSq);
    UndoMove(p, m, u);
    return total;
}

int EvaluationGlue(Position &p, const Move &m)
{
    const auto us = p.whoseturn;
    const auto u = MakeMove(p, m);
    int total = 0;
    auto outer = p.occupancy[us];
    while (outer) {
        const auto i = std::countr_zero(outer);
        outer &= outer - 1;
        auto inner = outer;
        while (inner) {
            const auto j = std::countr_zero(inner);
            inner &= inner - 1;
            total += Chebyshev(i, j);
        }
    }
    UndoMove(p, m, u);
    return -total;
}
