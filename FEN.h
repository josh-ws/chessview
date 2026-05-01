#ifndef FEN_H
#define FEN_H

#include "Bitboard.h"
#include <string>

Position ParseFEN(const std::string &str)
{
    return CreateDefaultPosition(); // TODO
}

std::string ToFEN(const Position &position)
{
    return "";
}

#endif
