#include "Perft.h"
#include "Bitboard.h"

uint64_t Perft(Position &p, int depth)
{
    if (depth <= 0)
        return 1;

    const auto moves = GenerateMoves(p);
    if (depth == 1)
        return moves.size();

    auto t = std::uint64_t(0);

    for (const auto &move : moves) {
        const auto u = MakeMove(p, move);
        t += Perft(p, depth - 1);
        UndoMove(p, move, u);
    }

    return t;
}
