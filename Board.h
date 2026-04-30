#ifndef BOARD_H
#define BOARD_H

#include "Move.h"
#include "Types.h"
#include <array>
#include <math.h>
#include <raylib.h>
#include <stdint.h>
#include <vector>

struct Change {
    u8 index;
    u8 value;
};

struct Undo {
    u8 bits;
    std::array<u8, 4> king;
    std::array<Change, 5> changes;
    int nChanges;
};

// Holds board state.
// Board state is held in m_pieces.
struct Board {
    Board();
    Board(const Board &other);
    auto operator=(const Board &other) = delete;

    static Board Default();

    inline u8 WhoseTurn() const noexcept { return !(m_bits & BLACKMOVE_MASK) ? PIECE_WHITE : PIECE_BLACK; }

    u8 pieceAt(u8 column, u8 row) const;
    u8 colorAt(u8 column, u8 row) const;
    Undo MakeNewMove(const Move &move);
    void UndoMove(const Undo &undo);
    auto getMoves(u8 count = 0) -> std::vector<Move>;
    bool isLegal(const Move &move);
    auto isCheck(u8 color) -> bool;
    auto isAttacked(u8 column, u8 row) const -> bool;
    auto isAttacked(u8 column, u8 row, u8 piece) const -> bool;
    auto getBoardState(uint16_t staleHalfMoveClock) -> BoardState;
    auto canCastle(u8 color, bool queenSide) -> bool;
    inline u8 enPassantColumn() {
        if (!(m_bits & DOUBLE_MASK))
            return 0xFF;
        return (m_bits & PAWN_MASK) >> 2;
    }

  private:
    std::array<u8, GRID_LENGTH * GRID_LENGTH> m_pieces;
    std::array<u8, 4> m_kingPos;
    u8 m_bits = 0U;

    auto setPiece(u8 piece, u8 column, u8 row) -> void;
    auto removePiece(u8 column, u8 row) -> void;

    auto isEnPassant(const Move &move) const -> bool;
    auto isStale() -> bool;
    auto isMoveIntoCheck(const Move &move) -> bool;
    auto isMoveLegalForPiece(u8 piece, const Move &move) -> bool;
};

#endif
