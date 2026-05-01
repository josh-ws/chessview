#include "Perft.h"
#include "Bitboard.h"

uint64_t Perft(Position &p, int depth) {
    if (depth <= 0)
        return 1;

    std::array<Move, MAX_MOVES> moves;
    int nmoves = GenerateMoves(p, moves);

    if (depth == 1)
        return nmoves;

    auto t = std::uint64_t(0);

    for (int i = 0; i < nmoves; i++) {
        const auto u = MakeMove(p, moves[i]);
        t += Perft(p, depth - 1);
        UndoMove(p, moves[i], u);
    }

    return t;
}
