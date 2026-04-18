#include "Perft.h"

std::uint64_t Perft(Board &board, int depth) {
    if (depth <= 0)
        return 1;

    const auto moves = board.getMoves();
    if (depth == 1)
        return moves.size();

    std::uint64_t total = 0;
    for (const auto &move : moves) {
        total += board.tryMove(
            move, [&board, depth]() { return Perft(board, depth - 1); });
    }
    return total;
}
