#ifndef BOARD_H
#define BOARD_H

#include "Move.h"
#include "Types.h"
#include <array>
#include <iostream>
#include <math.h>
#include <raylib.h>
#include <stdint.h>
#include <unordered_map>
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

    inline u8 WhoseTurn() const noexcept { return whiteMove() ? PIECE_WHITE : PIECE_BLACK; }

    // Resets the board so there are no pieces, and all flags reset.
    auto reset() -> void;

    // Returns the piece encoded as a u8 at the specified column and row.
    // Bitwise magic with PieceBits is required to extract information.
    auto pieceAt(u8 column, u8 row) const -> u8;

    // Returns the colour of the specified tile. (WHITE or BLACK)
    auto colorAt(u8 column, u8 row) const -> u8;

    // Sets the piece at the specified position to the specified piece.
    // piece must bitwise AND with exactly one type and one colour;
    // otherwise, the behaviour is undefined.
    auto setPiece(u8 piece, u8 column, u8 row) -> void;

    // Blanks out a piece at the specified position (set to 0).
    auto removePiece(u8 column, u8 row) -> void;

    auto isMoveLegal(const Move &move) -> bool;
    Undo MakeNewMove(const Move &move);
    void UndoMove(const Undo &undo);

    // Returns the number of moves for the specified player (color) WHITE or
    // BLACK. Specify count as non-zero to restrict the result to [count]
    // moves only. For example, count = 1 returns exactly one move (or zero
    // if none legal).
    auto getMoves(u8 count = 0) -> std::vector<Move>;

    // Returns whether the specified player (WHITE or BLACK) has exactly
    // zero moves remaining. (For checkmate and stalemate situations)
    auto hasZeroMoves() -> bool;

    // Returns whether the specified player has exactly one move remaining.
    // (For stalemate situations).
    auto hasOneMove() -> bool;

    // Returns whether the specified player is currently in check (King is
    // attacked by a piece).
    auto isCheck(u8 color) -> bool;

    // Returns whether the tile is attacked by an opponent's piece.
    // Note we don't specify a player here; the piece at the target is taken
    // as the defender.
    auto isAttacked(u8 column, u8 row) const -> bool;

    // Same as above, but specifying a piece that, if it were on that
    // square, would be attacked.
    auto isAttacked(u8 column, u8 row, u8 piece) const -> bool;

    // Returns true if the move is an en passant capture.
    auto isEnPassant(const Move &move) const -> bool;

    // Returns the current board state, depending on whose turn it is.
    auto getBoardState(uint16_t staleHalfMoveClock) -> BoardState;

    // Returns true if the specified player has moved their king. (For
    // castling rules.)
    auto kingMoved(u8 player) const -> bool;

    // Returns true if it is white's move, otherwise false.
    auto whiteMove() const -> bool;

    // Returns the column where an en passant is possible. Returns 255 if
    // invalid.
    auto enPassantColumn() const -> u8;

    // Returns castling possibility.
    auto canCastle(u8 color, bool queenSide) -> bool;

    // Returns whether the last move was "stale", i.e. no pawn move and no
    // capture
    auto isStale() -> bool;

  protected:
    std::array<u8, GRID_LENGTH * GRID_LENGTH> m_pieces;

    // Board state bits.
    // Bit 0 is used for when it is black's move. (True = black's move)
    // Bit 1 is used when a pawn has made a double move. This is for en
    // passant captures. Bits 2-5 are used for the column of the double pawn
    // move. This is for en passant captures. Bit 6 is used to denote that
    // the last move was "stale".
    u8 m_bits = 0U;

    std::array<u8, 4> m_kingPos;

    // Checks whether the specified column and row is within the confines of
    // the board.
    auto inBounds(u8 column, u8 row) const -> bool;

    // Checks whether the source tile and target tile of the move is within
    // the confines of the board.
    auto inBounds(const Move &move) const -> bool;

    // Checks whether the piece can move in the manner specified. For
    // example, no pawns moving 3 tiles, no kings moving 2 tiles, etc.
    auto isMoveLegalForPiece(u8 piece, const Move &move) -> bool;

    // Returns whether the specified move would leave the player in check.
    // This is an illegal move.
    auto isMoveIntoCheck(const Move &move) -> bool;
};

Board CreateDefaultBoard();

#endif
