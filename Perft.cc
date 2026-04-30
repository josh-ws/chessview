#include "Perft.h"

uint64_t Perft(Position &p, int depth) {
    if (depth <= 0)
        return 1;

    std::array<Move, MAX_MOVES> moves;
    int nmoves = GenerateMoves(p, moves);

    if (depth == 1)
        return nmoves;

    auto t = std::uint64_t(0);

#pragma omp parallel for reduction(+ : t) schedule(dynamic, 1)
    for (int i = 0; i < nmoves; i++) {
        Position np = p;
        MakeMove(np, moves[i]);
        t += Perft(np, depth - 1);
    }

    return t;
}
