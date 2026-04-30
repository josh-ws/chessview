#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

using u8 = uint_least8_t;
using u16 = uint_least16_t;
using u32 = uint_least32_t;

// Board state.
enum BoardState {
    STATE_NORMAL,
    STATE_CHECKMATE,
    STATE_STALEMATE,
    STATE_FORCED_DRAW_INSUFFICIENT_MATERIAL,
    STATE_FORCED_DRAW_FIFTY_MOVES,
};

// Length of the grid. Grid lengths of different sizes (up to 16!) should work,
// but untested.
const static u8 GRID_LENGTH = 8;

#endif
