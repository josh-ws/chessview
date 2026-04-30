#pragma once

#include <array>
#include <cstdint>

static constexpr int MAX_MOVES = 218; // https://chess.stackexchange.com/questions/4490/maximum-possible-movement-in-a-turn
static constexpr int NO_EP = 64;

enum Color : uint8_t {
    CWHITE,
    CBLACK,
};

enum PieceType : uint8_t {
    PAWN,
    KING,
    QUEEN,
    BISHOP,
    KNIGHT,
    ROOK,
    NONE,
};

enum Castling : uint8_t {
    CR_WK = 1 << 0,
    CR_WQ = 1 << 1,
    CR_BK = 1 << 2,
    CR_BQ = 1 << 3,
};

using Piece = uint8_t;

inline constexpr Piece MakePiece(Color c, PieceType p) { return static_cast<Piece>((c << 3) | p); }
inline constexpr PieceType TypeOf(Piece p) { return static_cast<PieceType>(p & 7); }
inline constexpr Color ColorOf(Piece p) { return static_cast<Color>(p >> 3); }

enum MoveFlag : uint8_t {
    MV_EP = 1,
    MV_DOUBLE = 2,
    MV_CASTLING = 4,
};

struct Move {
    uint8_t from;
    uint8_t to;
    uint8_t piece;
    uint8_t flags;
    uint8_t promo = NONE;
};

struct Position {
    uint64_t bitboards[2][6]{};
    uint64_t occupancy[2]{};
    Color whoseturn;
    uint8_t epsq;
    uint8_t castling;
};

constexpr Position CreateDefaultPosition()
{
    auto p = Position();
    p.bitboards[CWHITE][PAWN] = 0b00000000'00000000'00000000'00000000'00000000'00000000'11111111'00000000ULL;
    p.bitboards[CWHITE][KING] = 0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00010000ULL;
    p.bitboards[CWHITE][QUEEN] = 0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00001000ULL;
    p.bitboards[CWHITE][BISHOP] = 0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00100100ULL;
    p.bitboards[CWHITE][KNIGHT] = 0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'01000010ULL;
    p.bitboards[CWHITE][ROOK] = 0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'10000001ULL;
    p.bitboards[CBLACK][PAWN] = 0b00000000'11111111'00000000'00000000'00000000'00000000'00000000'00000000ULL;
    p.bitboards[CBLACK][KING] = 0b00010000'00000000'00000000'00000000'00000000'00000000'00000000'00000000ULL;
    p.bitboards[CBLACK][QUEEN] = 0b00001000'00000000'00000000'00000000'00000000'00000000'00000000'00000000ULL;
    p.bitboards[CBLACK][BISHOP] = 0b00100100'00000000'00000000'00000000'00000000'00000000'00000000'00000000ULL;
    p.bitboards[CBLACK][KNIGHT] = 0b01000010'00000000'00000000'00000000'00000000'00000000'00000000'00000000ULL;
    p.bitboards[CBLACK][ROOK] = 0b10000001'00000000'00000000'00000000'00000000'00000000'00000000'00000000ULL;
    p.occupancy[CWHITE] = p.bitboards[CWHITE][PAWN] | p.bitboards[CWHITE][KING] | p.bitboards[CWHITE][QUEEN] | p.bitboards[CWHITE][BISHOP] | p.bitboards[CWHITE][KNIGHT] | p.bitboards[CWHITE][ROOK];
    p.occupancy[CBLACK] = p.bitboards[CBLACK][PAWN] | p.bitboards[CBLACK][KING] | p.bitboards[CBLACK][QUEEN] | p.bitboards[CBLACK][BISHOP] | p.bitboards[CBLACK][KNIGHT] | p.bitboards[CBLACK][ROOK];
    p.epsq = NO_EP;
    p.castling = CR_WK | CR_WQ | CR_BK | CR_BQ;
    return p;
}

void InitLookupTables();
int GenerateMoves(Position &p, std::array<Move, MAX_MOVES> &moves);
void MakeMove(Position &p, const Move &m);
