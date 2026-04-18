#ifndef PERFT_H
#define PERFT_H

#include "Board.h"
#include <cstdint>

std::uint64_t Perft(Board &board, int depth);

#endif
