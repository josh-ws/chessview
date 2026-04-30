#include "Bitboard.h"
#include <array>
#include <bit>
#include <byteswap.h>
#include <cassert>
#include <cstdint>

inline constexpr uint64_t BitOf(uint8_t col, uint8_t row) { return 1ULL << (row * 8 + col); }
inline constexpr int ColOf(int bit) { return bit & 7; }
inline constexpr int RowOf(int bit) { return bit >> 3; }

static constexpr uint64_t RANK_1 = 0x00000000000000FFULL;
static constexpr uint64_t RANK_2 = 0x000000000000FF00ULL;
static constexpr uint64_t RANK_3 = 0x0000000000FF0000ULL;
static constexpr uint64_t RANK_4 = 0x00000000FF000000ULL;
static constexpr uint64_t RANK_5 = 0x000000FF00000000ULL;
static constexpr uint64_t RANK_6 = 0x0000FF0000000000ULL;
static constexpr uint64_t RANK_7 = 0x00FF000000000000ULL;
static constexpr uint64_t RANK_8 = 0xFF00000000000000ULL;
static constexpr uint64_t FILE_A = 0x0101010101010101ULL;
static constexpr uint64_t FILE_B = 0x0202020202020202ULL;
static constexpr uint64_t FILE_C = 0x0404040404040404ULL;
static constexpr uint64_t FILE_D = 0x0808080808080808ULL;
static constexpr uint64_t FILE_E = 0x1010101010101010ULL;
static constexpr uint64_t FILE_F = 0x2020202020202020ULL;
static constexpr uint64_t FILE_G = 0x4040404040404040ULL;
static constexpr uint64_t FILE_H = 0x8080808080808080ULL;

static std::array<uint64_t, 64> KNIGHT_ATTACKS{};
static std::array<uint64_t, 64> KING_ATTACKS{};
static std::array<std::array<uint8_t, 64>, 8> FIRST_RANK_ATTACKS{};
static std::array<uint8_t, 64> CASTLE_MASK;

static std::array<uint64_t, 64> DIAG;
static std::array<uint64_t, 64> ANTIDIAG;
static std::array<uint64_t, 8> RANKS = {RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8};
static std::array<uint64_t, 8> FILES = {FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H};

constexpr static bool OnBoard(int col, int row)
{
    return col >= 0 && col < 8 && row >= 0 && row < 8;
}

static void InitFirstRankAttacks()
{
    for (int col = 0; col < 8; col++)
        for (int rocc = 0; rocc < 64; rocc++) {
            const auto occ = uint8_t(rocc) << 1;
            auto atk = 0;
            for (int c = col + 1; c < 8; c++) {
                atk |= 1u << c;
                if (occ & (1u << c))
                    break;
            }
            for (int c = col - 1; c >= 0; c--) {
                atk |= 1u << c;
                if (occ & (1u << c))
                    break;
            }
            FIRST_RANK_ATTACKS[col][rocc] = atk;
        }
}

static void InitDiag()
{
    for (int i = 0; i < 64; i++) {
        const auto col = ColOf(i), row = RowOf(i);
        auto d = uint64_t(0);
        auto a = uint64_t(0);
        // NE
        for (int dc = 1, dr = 1; OnBoard(col + dc, row + dr); dc++, dr++)
            d |= BitOf(col + dc, row + dr);
        for (int dc = -1, dr = -1; OnBoard(col + dc, row + dr); dc--, dr--)
            d |= BitOf(col + dc, row + dr);
        for (int dc = -1, dr = 1; OnBoard(col + dc, row + dr); dc--, dr++)
            a |= BitOf(col + dc, row + dr);
        for (int dc = 1, dr = -1; OnBoard(col + dc, row + dr); dc++, dr--)
            a |= BitOf(col + dc, row + dr);
        DIAG[i] = d;
        ANTIDIAG[i] = a;
    }
}

static void InitKnightAttacks()
{
    for (int i = 0; i < 64; i++) {
        uint64_t b = 1ULL << i;
        uint64_t a = 0;
        a |= (b & ~FILE_H) << 17;
        a |= (b & ~FILE_A) << 15;
        a |= (b & ~(FILE_G | FILE_H)) << 10;
        a |= (b & ~(FILE_A | FILE_B)) << 6;
        a |= (b & ~(FILE_G | FILE_H)) >> 6;
        a |= (b & ~(FILE_A | FILE_B)) >> 10;
        a |= (b & ~FILE_H) >> 15;
        a |= (b & ~FILE_A) >> 17;
        KNIGHT_ATTACKS[i] = a;
    }
}

static void InitKingAttacks()
{
    for (int i = 0; i < 64; i++) {
        uint64_t b = 1ULL << i;
        uint64_t a = 0;
        a |= (b & ~FILE_H) << 1;
        a |= (b & ~FILE_A) << 7;
        a |= b << 8;
        a |= (b & ~FILE_H) << 9;
        a |= (b & ~FILE_A) >> 1;
        a |= (b & ~FILE_H) >> 7;
        a |= b >> 8;
        a |= (b & ~FILE_A) >> 9;
        KING_ATTACKS[i] = a;
    }
}

static void InitCastleMask()
{
    CASTLE_MASK.fill(0xFF);
    CASTLE_MASK[4] = ~(CR_WK | CR_WQ);
    CASTLE_MASK[7] = ~CR_WK;
    CASTLE_MASK[0] = ~CR_WQ;
    CASTLE_MASK[60] = ~(CR_BK | CR_BQ);
    CASTLE_MASK[63] = ~CR_BK;
    CASTLE_MASK[56] = ~CR_BQ;
}

void InitLookupTables()
{
    InitFirstRankAttacks();
    InitKingAttacks();
    InitKnightAttacks();
    InitDiag();
    InitCastleMask();
}

inline constexpr int Lsb(uint64_t b) { return std::countr_zero(b); }
inline constexpr int PopLsb(uint64_t &b)
{
    auto sq = std::countr_zero(b);
    b &= b - 1;
    return sq;
}

static uint64_t LineAttacks(uint64_t occ, uint64_t mask, uint64_t r)
{
    const auto o = occ & mask;
    const auto fwd = (o - 2 * r);
    const auto rev = std::byteswap(std::byteswap(o) - 2 * std::byteswap(r));
    return (fwd ^ rev) & mask;
}

static uint64_t RankAttacks(uint64_t sq, uint64_t occ)
{
    const auto col = ColOf(sq);
    const auto row = RowOf(sq);
    const auto rocc = (occ >> (row * 8 + 1)) & 0x3F; // full 8-bit occupancy for that rank
    return (uint64_t(FIRST_RANK_ATTACKS[col][rocc]) << (row * 8));
}

static uint64_t BishopAttacks(int sq, uint64_t occ)
{
    const auto r = 1ULL << sq;
    return LineAttacks(occ, DIAG[sq], r) | LineAttacks(occ, ANTIDIAG[sq], r);
}

static uint64_t RookAttacks(int sq, uint64_t occ)
{
    const auto r = 1ULL << sq;
    return LineAttacks(occ, FILES[ColOf(sq)], r) | RankAttacks(sq, occ);
}

static uint64_t QueenAttacks(int sq, uint64_t occ)
{
    return BishopAttacks(sq, occ) | RookAttacks(sq, occ);
}

void MakeMove(Position &p, const Move &m)
{
    const auto mycolor = p.whoseturn;
    const auto theircolor = Color(mycolor ^ 1);
    const auto from = 1ULL << m.from;
    const auto to = 1ULL << m.to;
    const auto move = from ^ to;

    p.epsq = NO_EP;
    p.castling &= CASTLE_MASK[m.from] & CASTLE_MASK[m.to];
    p.bitboards[mycolor][m.piece] ^= move;
    p.occupancy[mycolor] ^= move;

    if (m.flags & MV_EP) {
        const auto cap = (mycolor == CWHITE) ? (to >> 8) : (to << 8);
        p.bitboards[theircolor][PAWN] ^= cap;
        p.occupancy[theircolor] ^= cap;
    }
    else if (m.flags & MV_DOUBLE) {
        p.epsq = (m.from + m.to) / 2;
    }
    else if (p.occupancy[theircolor] & to) {
        for (int pt = 0; pt < 6; pt++) {
            if (p.bitboards[theircolor][pt] & to) {
                p.bitboards[theircolor][pt] ^= to;
                break;
            }
        }
        p.occupancy[theircolor] ^= to;
    }

    if (m.flags & MV_CASTLING) {
        auto rookMove = uint64_t(0);
        switch (m.to) {
        case 6:
            rookMove = (1ULL << 7) | (1ULL << 5);
            break;
        case 2:
            rookMove = (1ULL << 0) | (1ULL << 3);
            break;
        case 62:
            rookMove = (1ULL << 63) | (1ULL << 61);
            break;
        case 58:
            rookMove = (1ULL << 56) | (1ULL << 59);
            break;
        }
        p.bitboards[mycolor][ROOK] ^= rookMove;
        p.occupancy[mycolor] ^= rookMove;
    }

    if (m.promo != NONE) {
        p.bitboards[mycolor][PAWN] ^= to;
        p.bitboards[mycolor][m.promo] ^= to;
    }

    p.whoseturn = theircolor;
}

static bool IsAttacked(const Position &p, int sq, uint8_t bycolor)
{
    const auto mycolor = bycolor ^ 1;
    const auto theircolor = bycolor;
    const auto bitboard = 1ULL << sq;
    const auto all = p.occupancy[CWHITE] | p.occupancy[CBLACK];

    const auto pawns = p.bitboards[theircolor][PAWN];
    if (mycolor == CWHITE) {
        // enemy (black) pawns attack via >>7 and >>9
        auto capR = (pawns >> 7) & ~FILE_A;
        auto capL = (pawns >> 9) & ~FILE_H;
        if ((capR & bitboard) || (capL & bitboard))
            return true;
    }
    else {
        // enemy (white) pawns attack via <<7 and <<9
        auto capR = (pawns << 9) & ~FILE_A;
        auto capL = (pawns << 7) & ~FILE_H;
        if ((capR & bitboard) || (capL & bitboard))
            return true;
    }

    auto knights = p.bitboards[theircolor][KNIGHT];
    while (knights) {
        auto from = PopLsb(knights);
        auto targets = KNIGHT_ATTACKS[from];
        if (targets & bitboard)
            return true;
    }

    auto bishops = p.bitboards[theircolor][BISHOP];
    while (bishops) {
        auto from = PopLsb(bishops);
        auto targets = BishopAttacks(from, all);
        if (targets & bitboard)
            return true;
    }

    auto rooks = p.bitboards[theircolor][ROOK];
    while (rooks) {
        auto from = PopLsb(rooks);
        auto targets = RookAttacks(from, all);
        if (targets & bitboard)
            return true;
    }

    auto queens = p.bitboards[theircolor][QUEEN];
    while (queens) {
        auto from = PopLsb(queens);
        auto targets = QueenAttacks(from, all);
        if (targets & bitboard)
            return true;
    }

    auto king = Lsb(p.bitboards[theircolor][KING]);
    auto targets = KING_ATTACKS[king];
    if (targets & bitboard)
        return true;

    return false;
}

static bool IsCheck(const Position &p, uint8_t color)
{
    return IsAttacked(p, Lsb(p.bitboards[color][KING]), color ^ 1);
}

static bool CheckLegal(Position &p, const Move &m)
{
    const auto mycolor = p.whoseturn;

    auto newp = p;
    MakeMove(newp, m);
    return !IsCheck(newp, mycolor);
}

int GenerateMoves(Position &p, std::array<Move, MAX_MOVES> &moves)
{
    const auto mycolor = p.whoseturn;
    const auto theircolor = p.whoseturn ^ CBLACK;
    const auto empty = ~(p.occupancy[CWHITE] | p.occupancy[CBLACK]);
    const auto all = p.occupancy[CWHITE] | p.occupancy[CBLACK];
    const auto lastrank = (mycolor == CWHITE) ? RANK_8 : RANK_1;

    int index = 0;

    const auto emit = [&](uint8_t from, uint8_t to, uint8_t piece, uint8_t flags = 0, uint8_t promo = NONE) {
        moves[index].from = from;
        moves[index].to = to;
        moves[index].piece = piece;
        moves[index].flags = flags;
        moves[index].promo = promo;
        if (CheckLegal(p, moves[index]))
            index += 1;
    };

    const auto doTargets = [&](uint64_t targets, int add, uint8_t flags = 0) {
        while (targets) {
            auto to = PopLsb(targets);
            emit(to + add, to, PAWN, flags);
        }
    };

    const auto doPromos = [&](uint64_t targets, int add) {
        while (targets) {
            auto to = PopLsb(targets);
            for (auto pt : {QUEEN, ROOK, BISHOP, KNIGHT}) {
                emit(to + add, to, PAWN, 0, pt);
            }
        }
    };

    const auto pawns = p.bitboards[mycolor][PAWN];
    if (mycolor == CWHITE) {
        const auto single = (pawns << 8) & empty;                           // single pawn moves
        const auto dbl = ((single & RANK_3) << 8) & empty;                  // double pawn moves
        const auto capR = (pawns << 9) & ~FILE_A & p.occupancy[theircolor]; // captures (left)
        const auto capL = (pawns << 7) & ~FILE_H & p.occupancy[theircolor]; // captures (right)

        // quiet moves
        doTargets(dbl, -16, MV_DOUBLE);
        doTargets(single & ~lastrank, -8);
        doTargets(capR & ~lastrank, -9);
        doTargets(capL & ~lastrank, -7);
        // promotions
        doPromos(single & lastrank, -8);
        doPromos(capR & lastrank, -9);
        doPromos(capL & lastrank, -7);
    }
    else {
        auto single = (pawns >> 8) & empty;                           // single pawn moves
        auto dbl = ((single & RANK_6) >> 8) & empty;                  // double pawn moves
        auto capR = (pawns >> 9) & ~FILE_H & p.occupancy[theircolor]; // captures (left)
        auto capL = (pawns >> 7) & ~FILE_A & p.occupancy[theircolor]; // captures (right)

        // quiet moves
        doTargets(dbl, 16, MV_DOUBLE);
        doTargets(single & ~lastrank, 8);
        doTargets(capR & ~lastrank, 9);
        doTargets(capL & ~lastrank, 7);
        // promotions
        doPromos(single & lastrank, 8);
        doPromos(capR & lastrank, 9);
        doPromos(capL & lastrank, 7);
    }

    if (p.epsq != NO_EP) {
        const uint64_t b = 1ULL << p.epsq;
        if (mycolor == CWHITE) {
            auto epR = (pawns << 9) & ~FILE_A & b;
            auto epL = (pawns << 7) & ~FILE_H & b;
            if (epR)
                emit(p.epsq - 9, p.epsq, PAWN, MV_EP);
            if (epL)
                emit(p.epsq - 7, p.epsq, PAWN, MV_EP);
        }
        else {
            auto epR = (pawns >> 9) & ~FILE_H & b;
            auto epL = (pawns >> 7) & ~FILE_A & b;
            if (epR)
                emit(p.epsq + 9, p.epsq, PAWN, MV_EP);
            if (epL)
                emit(p.epsq + 7, p.epsq, PAWN, MV_EP);
        }
    }

    auto knights = p.bitboards[mycolor][KNIGHT];
    while (knights) {
        auto from = PopLsb(knights);
        auto targets = KNIGHT_ATTACKS[from] & ~p.occupancy[mycolor];
        while (targets)
            emit(from, PopLsb(targets), KNIGHT);
    }

    auto bishops = p.bitboards[mycolor][BISHOP];
    while (bishops) {
        auto from = PopLsb(bishops);
        auto targets = BishopAttacks(from, all) & ~p.occupancy[mycolor];
        while (targets)
            emit(from, PopLsb(targets), BISHOP);
    }

    auto rooks = p.bitboards[mycolor][ROOK];
    while (rooks) {
        auto from = PopLsb(rooks);
        auto targets = RookAttacks(from, all) & ~p.occupancy[mycolor];
        while (targets)
            emit(from, PopLsb(targets), ROOK);
    }

    auto queens = p.bitboards[mycolor][QUEEN];
    while (queens) {
        auto from = PopLsb(queens);
        auto targets = QueenAttacks(from, all) & ~p.occupancy[mycolor];
        while (targets)
            emit(from, PopLsb(targets), QUEEN);
    }

    auto kings = p.bitboards[mycolor][KING];
    while (kings) {
        auto from = PopLsb(kings);
        auto targets = KING_ATTACKS[from] & ~p.occupancy[mycolor];
        while (targets)
            emit(from, PopLsb(targets), KING);
    }

    // castling
    if (mycolor == CWHITE) {
        if (p.castling & CR_WK) {
            const auto empty = !(all & BitOf(5, 0)) && !(all & BitOf(6, 0));
            auto attacked = false;
            for (int i = 4; i <= 6; i++) {
                if (IsAttacked(p, i, CBLACK))
                    attacked = true;
            }
            if (empty && !attacked) {
                emit(4, 6, KING, MV_CASTLING);
            }
        }
        if (p.castling & CR_WQ) {
            const auto empty = !(all & BitOf(3, 0)) && !(all & BitOf(2, 0)) && !(all & BitOf(1, 0));
            auto attacked = false;
            for (int i = 2; i <= 4; i++) {
                if (IsAttacked(p, i, CBLACK))
                    attacked = true;
            }
            if (empty && !attacked) {
                emit(4, 2, KING, MV_CASTLING);
            }
        }
    }
    else {
        if (p.castling & CR_BK) {
            const auto empty = !(all & BitOf(5, 7)) && !(all & BitOf(6, 7));
            auto attacked = false;
            for (int i = 60; i <= 62; i++) {
                if (IsAttacked(p, i, CWHITE))
                    attacked = true;
            }
            if (empty && !attacked) {
                emit(60, 62, KING, MV_CASTLING);
            }
        }
        if (p.castling & CR_BQ) {
            const auto empty = !(all & BitOf(3, 7)) && !(all & BitOf(2, 7)) && !(all & BitOf(1, 7));
            auto attacked = false;
            for (int i = 58; i <= 60; i++) {
                if (IsAttacked(p, i, CWHITE))
                    attacked = true;
            }
            if (empty && !attacked) {
                emit(60, 58, KING, MV_CASTLING);
            }
        }
    }

    return index;
}
