#include <algorithm>
#include <bit>
#include <cstdlib>
#include <functional>

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
    for (const auto piece : {QUEEN, ROOK, BISHOP, KNIGHT, KING, PAWN}) {
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

int EvaluationWhiteSquares(Position &p, const Move &m)
{
    const auto us = p.whoseturn;
    const auto u = MakeMove(p, m);
    auto b = p.occupancy[us];
    auto count = 0;
    while (b) {
        const auto sq = std::countr_zero(b);
        b &= b - 1;
        if ((ColOf(sq) + RowOf(sq)) % 2 == 1)
            count += 1;
    }
    UndoMove(p, m, u);
    return count;
}

static int MirrorX(int sq) { return ColOf(sq) + (7 - RowOf(sq)) * 8; }
static int MirrorY(int sq) { return (7 - ColOf(sq)) + RowOf(sq) * 8; }
static int MirrorXY(int sq) { return (7 - ColOf(sq)) + (7 - RowOf(sq)) * 8; }

using MirrorFn = std::function<int(int)>;

static int EvaluationMirror(Position &p, const Move &m, auto mirrorFn)
{
    auto score = 0;
    const auto undo = MakeMove(p, m);
    for (int sq = 0; sq < 64; sq++) {
        const auto pc = GetPiece(p, ColOf(sq), RowOf(sq));
        const auto m = mirrorFn(sq);
        const auto mpc = GetPiece(p, ColOf(m), RowOf(m));
        if (pc == NONE || mpc == NONE)
            continue;
        if (TypeOf(pc) == TypeOf(mpc))
            score += 2;
        else
            score += 1;
    }
    UndoMove(p, m, undo);
    return score;
}

int EvaluationMirrorX(Position &p, const Move &m)
{
    return EvaluationMirror(p, m, MirrorX);
}

int EvaluationMirrorY(Position &p, const Move &m)
{
    return EvaluationMirror(p, m, MirrorY);
}

int EvaluationMirrorXY(Position &p, const Move &m)
{
    return EvaluationMirror(p, m, MirrorXY);
}

int EvaluationCenter(Position &p, const Move &m)
{
    const auto us = p.whoseturn;
    const auto undo = MakeMove(p, m);

    auto score = 0;
    auto b = p.occupancy[us];
    while (b) {
        const auto sq = std::countr_zero(b);
        b &= b - 1;
        if (ColOf(sq) >= 3 && ColOf(sq) <= 4 && RowOf(sq) >= 3 && RowOf(sq) <= 4)
            score += 2;
        else if (ColOf(sq) >= 2 && ColOf(sq) <= 5 && RowOf(sq) >= 2 && RowOf(sq) <= 5)
            score += 1;
    }
    UndoMove(p, m, undo);
    return score;
}

int EvaluationEdge(Position &p, const Move &m)
{
    const auto us = p.whoseturn;
    const auto undo = MakeMove(p, m);

    auto score = 0;
    auto b = p.occupancy[us];
    while (b) {
        const auto sq = std::countr_zero(b);
        b &= b - 1;
        if ((ColOf(sq) == 0 || ColOf(sq) == 7))
            score += 1;
    }
    UndoMove(p, m, undo);
    return score;
}

// minimizes number of opponent responses
int EvaluationMinResponse(Position &p, const Move &m)
{
    const auto undo = MakeMove(p, m);
    auto moves = GenerateMoves(p);
    UndoMove(p, m, undo);
    return -moves.size();
}

// minimizes number of own moves
int EvaluationMinSelf(Position &p, const Move &m)
{
    const auto undo = MakeMove(p, m);
    auto copy = p; // explicitly copy the position, because...
    UndoMove(p, m, undo);
    copy.whoseturn = static_cast<CColor>(copy.whoseturn ^ 1); // ...we need to flip whose turn
    return -GenerateMoves(copy).size();
}
