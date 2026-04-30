#ifndef FEN_H
#define FEN_H

#include "Bitboard.h"
#include <string>

Position ParseFEN(const std::string &str) {
    return CreateDefaultPosition(); // TODO
}

std::string ToFEN(const Position &position) {
    static constexpr char chars[6] = {'p', 'k', 'q', 'b', 'n', 'r'};
    std::string out;
    for (int rank = 7; rank >= 0; --rank) {
        int empty = 0;
        for (int file = 0; file < 8; ++file) {
            uint64_t bit = 1ULL << (rank * 8 + file);
            char c = 0;
            for (int color = 0; color < 2 && !c; ++color) {
                for (int pt = 0; pt < 6; ++pt) {
                    if (position.bitboards[color][pt] & bit) {
                        c = color == CWHITE ? chars[pt] - 32 : chars[pt];
                        break;
                    }
                }
            }
            if (c) {
                if (empty) { out += char('0' + empty); empty = 0; }
                out += c;
            } else {
                ++empty;
            }
        }
        if (empty) out += char('0' + empty);
        if (rank) out += '/';
    }
    out += position.whoseturn == CWHITE ? " w " : " b ";
    out += "- - 0 1";
    return out;
}

#endif
