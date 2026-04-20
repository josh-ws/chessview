#include "Board.h"
#include "Types.h"
#include <cstdlib>

Board CreateDefaultBoard() {
    auto b = Board();

    // Pawns
    for (int row : {1, 6})
        for (int column = 0; column < 8; ++column) {
            u8 color = row == 1 ? PIECE_WHITE : PIECE_BLACK;
            b.setPiece(PIECE_PAWN | color, column, row);
        }

    for (int row : {0, 7}) {
        u8 color = row == 0 ? PIECE_WHITE : PIECE_BLACK;
        b.setPiece(PIECE_QUEEN | color, 3, row);
        b.setPiece(PIECE_KING | color, 4, row);
        for (int columnMultiplier : {0, 1}) {
            b.setPiece(PIECE_BISHOP | color, 2 + (3 * columnMultiplier), row);
            b.setPiece(PIECE_KNIGHT | color, 1 + (5 * columnMultiplier), row);
            b.setPiece(PIECE_CASTLE | color, 0 + (7 * columnMultiplier), row);
        }
    }
    return b;
}

Board::Board() : m_pieces{}, m_kingPos() {}

Board::Board(const Board &other) : m_pieces{}, m_kingPos() {
    m_pieces = other.m_pieces;
    m_bits = other.m_bits;
    m_kingPos = other.m_kingPos;
}

auto Board::reset() -> void {
    m_pieces.fill(0);
    m_bits = 0;
    m_kingPos.fill(0);
}

auto Board::pieceAt(u8 column, u8 row) const -> u8 {
    return m_pieces[column * GRID_LENGTH + row];
}

auto Board::colorAt(u8 column, u8 row) const -> u8 {
    // Odd numbered tiles are white. (0, 0) is black, (1, 0) is white, and
    // so forth.
    return (column + row) & 1 ? PIECE_WHITE : PIECE_BLACK;
}

auto Board::setPiece(u8 piece, u8 column, u8 row) -> void {
    m_pieces[column * GRID_LENGTH + row] = piece;
    if ((piece & TYPE_MASK) == PIECE_KING) {
        const u8 idx = ((piece & COLOR_MASK) >> 3) * 2;
        m_kingPos[idx] = column;
        m_kingPos[idx + 1] = row;
    }
}

auto Board::removePiece(u8 column, u8 row) -> void {
    // Mask is set to PIECE_MASK (0b1111) shifted by column * 4. So if
    // column = 2 then mask = 0b111100000000.
    m_pieces[column * GRID_LENGTH + row] = EMPTY;
}

auto Board::inBounds(u8 column, u8 row) const -> bool {
    return column < GRID_LENGTH && row < GRID_LENGTH;
}

auto Board::inBounds(const Move &move) const -> bool {
    // Both source and target tile must be in bounds, otherwise the move is
    // illegal.
    return inBounds(move.fromCol, move.fromRow) &&
           inBounds(move.toCol, move.toRow);
}

auto Board::isMoveLegal(const Move &move) -> bool {
    // Whose turn is it?
    const u8 mycolor = whiteMove() ? PIECE_WHITE : PIECE_BLACK;

    // Move not in bounds, so is immediately illegal
    if (!inBounds(move))
        return false;

    // Move that does nothing
    else if (move.fromCol == move.toCol && move.fromRow == move.toRow)
        return false;

    // Check source
    u8 piece = pieceAt(move.fromCol, move.fromRow);
    if (piece == EMPTY) // No piece at source, so nothing to move
        return false;
    else if ((piece & COLOR_MASK) != mycolor) // Piece at source cannot be moved
                                              // by the provided player
        return false;
    else if ((piece & TYPE_MASK) != PIECE_PAWN &&
             move.promotion) // Promoting a non pawn
        return false;

    // Check target
    u8 target = pieceAt(move.toCol, move.toRow);
    if (target &&
        (target & COLOR_MASK) == mycolor) // Capture is against same color piece
        return false;
    // Capture is against the King (technically the king cannot be captured)
    else if ((target & TYPE_MASK) == PIECE_KING)
        return false;

    return isMoveLegalForPiece(piece, move) && !isMoveIntoCheck(move);
}

auto Board::isEnPassant(const Move &move) const -> bool {
    if (!(m_bits & DOUBLE_MASK))
        return false;

    const u8 column = (m_bits & PAWN_MASK) >> 2;
    const u8 piece = pieceAt(move.fromCol, move.fromRow);

    // Moving piece must be a pawn
    if ((piece & TYPE_MASK) != PIECE_PAWN)
        return false;

    // Must be trying to take on the same column as the last move
    if (move.toCol != column)
        return false;

    switch (piece & COLOR_MASK) {
    case PIECE_BLACK:
        return move.fromRow == 3 && move.toRow == 2;
    case PIECE_WHITE:
        return move.fromRow == 4 && move.toRow == 5;
    }
    return false; // to stop warnings, but this should never be hit
}

auto Board::isMoveLegalForPiece(u8 piece, const Move &move) -> bool {
    // Can't move nothing
    if (piece == EMPTY)
        return false;

    const u8 dcol = abs(move.fromCol - move.toCol);
    const u8 drow = abs(move.fromRow - move.toRow);

    // Returns -1 if the direction is to the left or down, or 1 otherwise
    auto getDir = [](u8 from, u8 to) -> int { return from < to ? 1 : -1; };

    // Returns whether the piece is blocked on the diagonal vector.
    // Walks all tiles along the diagonal vector and checks for a piece in
    // the way. Note that we cannot "move through" an opponent's piece and
    // capture on the way - we get blocked by any piece.
    auto isBlockedOnDiagonal = [&]() -> bool {
        int columnDir = getDir(move.fromCol, move.toCol);
        int rowDir = getDir(move.fromRow, move.toRow);
        // Check every tile on this diagonal vector until we reach our
        // target.
        for (int col = move.fromCol + columnDir, row = move.fromRow + rowDir;
             col != move.toCol && row != move.toRow;
             col += columnDir, row += rowDir) {
            if (pieceAt(col, row))
                return true;
        }
        return false;
    };

    // Returns whether the piece is blocked on the vertical/horizontal
    // vector. Walks all tiles along the vertical or horizontal vector and
    // checks for a piece in the way. Note that we cannot "move through" an
    // opponent's piece and capture on the way - we get blocked by any
    // piece.
    auto isBlockedOnAxis = [&]() -> bool {
        if (drow) { // Moving vertically
            int dir = getDir(move.fromRow, move.toRow);
            for (int row = move.fromRow + dir; row != move.toRow; row += dir)
                if (pieceAt(move.fromCol, row))
                    return true;
        } else { // Moving horizontally
            int dir = getDir(move.fromCol, move.toCol);
            for (int col = move.fromCol + dir; col != move.toCol; col += dir)
                if (pieceAt(col, move.fromRow))
                    return true;
        }
        return false;
    };

    // Not moving
    if (!dcol && !drow)
        return false;

    switch (piece & TYPE_MASK) {
    case PIECE_BISHOP:
        // Bishop can only move diagonally, so the delta row MUST equal
        // the delta column (otherwise we're moving staggered)
        return dcol == drow && !isBlockedOnDiagonal();
    case PIECE_KING:
        // King can only move one tile at a time, but in any direction
        if (dcol <= 1 && drow <= 1)
            return true;
        else if (drow == 0 && dcol == 2) {
            // Castling
            const int dir = getDir(move.fromCol, move.toCol);
            const u8 castleColumn = dir == -1 ? 0 : 7;
            const u8 castle = pieceAt(castleColumn, move.fromRow);

            if ((castle & TYPE_MASK) != PIECE_CASTLE)
                return false;

            // Cannot move from, through or into check (but into
            // check will be covered elsewhere, so we only need to
            // check from and through)
            if (isAttacked(move.fromCol, move.fromRow))
                return false;
            else if (isAttacked(move.fromCol + dir, move.toRow))
                return false;

            // Finally, all tiles between the king and the rook must
            // be EMPTY
            for (u8 col = move.fromCol + dir; col != castleColumn; col += dir)
                if (pieceAt(col, move.fromRow) != EMPTY)
                    return false;

            return true;
        }
        return false;
    case PIECE_KNIGHT:
        // Knight can only move in an L shape; one of the directions
        // must be 2 and one must be 1, exactly.
        return (dcol == 2 && drow == 1) || (dcol == 1 && drow == 2);
    case PIECE_PAWN: {
        // Bit more complex for pawns.
        // Pawns can only move diagonally when capturing, and only 2
        // tiles from the initial square.
        const u8 color = piece & COLOR_MASK;
        const int dir = getDir(move.fromRow, move.toRow);
        if (color == PIECE_WHITE && dir == -1) // White moving backwards
            return false;
        else if (color == PIECE_BLACK && dir == 1) // Same for black
            return false;

        if (move.promotion) // Attempting to promote a pawn - is it
                            // legal?
        {
            if (move.toRow != ((color == PIECE_WHITE) ? GRID_LENGTH - 1 : 0))
                return false;
            const u8 pro = move.promotion & TYPE_MASK;
            if (pro != PIECE_KNIGHT && pro != PIECE_QUEEN && pro != PIECE_BISHOP && pro != PIECE_ROOK)
                return false;
        }

        if (dcol) // Attacking
        {
            // Can only attack moving one row at a time, and can
            // never move more than one column
            if (dcol > 1 || drow != 1)
                return false;

            u8 target = pieceAt(move.toCol, move.toRow);
            if (target != EMPTY)
                return true; // Attacking moving 1 column and 1
                             // row, this is legal

            // If we're here, we're performing an attack on an empty
            // square. The only way this is legal is if this is an
            // en passant move...
            return isEnPassant(move);
        }

        // Not attacking, so we can only move one or two spaces
        if (drow > 2)
            return false;

        // Can only move two spaces if moving from initial square
        // For white, this is row 1; for black, this is row 6; (0 based)
        if (drow == 2 && ((color == PIECE_WHITE && move.fromRow != 1) ||
                          (color == PIECE_BLACK && move.fromRow != 6)))
            return false;

        // Not attacking, so if there is a piece at target then this is
        // an illegal move
        u8 target = pieceAt(move.toCol, move.toRow);
        if (target != EMPTY)
            return false;
        else if (drow == 2 &&
                 pieceAt(move.fromCol, move.fromRow + dir) != EMPTY)
            return false;

        return true;
    }
    case PIECE_QUEEN:
        // If moving diagonally, can only move row and column being the
        // same (straight diagonal)
        if (drow && dcol && drow != dcol)
            return false;
        return (drow && dcol) ? !isBlockedOnDiagonal() : !isBlockedOnAxis();
    case PIECE_ROOK:
    case PIECE_CASTLE:
        // Rook can only move sideways or vertically, so delta row must
        // be 0 or delta column must be 0, otherwise we are moving
        // diagonally
        return (!drow || !dcol) && !isBlockedOnAxis();
    default:
        return false;
    }
}

auto Board::isMoveIntoCheck(const Move &move) -> bool {
    const auto mycolor = WhoseTurn();
    const auto u = MakeNewMove(move);
    const auto ischeck = isCheck(mycolor);
    UndoMove(u);
    return ischeck;
}

auto Board::getBoardState(uint16_t staleHalfMoveClock) -> BoardState {
    const u8 mycolor = whiteMove() ? PIECE_WHITE : PIECE_BLACK;
    // zero moves
    if (hasZeroMoves()) {
        // if zero moves and in check, that's checkmate - otherwise
        // stalemate
        return isCheck(mycolor) ? STATE_CHECKMATE : STATE_STALEMATE;
    }
    // 50 moves without capture or pawn move (we compare with 100 because
    // stale moves is counted per half-turn)
    else if (staleHalfMoveClock >= 100U) {
        return STATE_FORCED_DRAW_FIFTY_MOVES;
    }
    // check all piece types on the board (except king)
    // if we only have kings, or king v king + bishop or king v king +
    // knight then it's a forced draw
    u8 numPieces = 0U;
    struct {
        u8 type;
        u8 color;
        u8 tileColor;
    } alivePiece[2];

    for (u8 i = 0; i < GRID_LENGTH * GRID_LENGTH && numPieces <= 1; ++i) {
        const u8 row = i / GRID_LENGTH;
        const u8 column = i % GRID_LENGTH;
        const u8 piece = pieceAt(column, row);
        if ((piece & TYPE_MASK) != PIECE_KING) {
            alivePiece[numPieces++] = {(u8)(piece & TYPE_MASK),
                                       (u8)(piece & COLOR_MASK),
                                       colorAt(column, row)};
        }
    }
    // no pieces but kings, or one piece and it's a knight/bishop
    if (numPieces == 0 || (numPieces == 1 && (alivePiece[0].type == PIECE_KNIGHT ||
                                              alivePiece[0].type == PIECE_BISHOP)))
        return STATE_FORCED_DRAW_INSUFFICIENT_MATERIAL;
    // two pieces, and both bishops of same color
    else if (numPieces == 2) {
        bool isBothBishop =
            alivePiece[0].type == PIECE_BISHOP && alivePiece[1].type == PIECE_BISHOP;
        bool isOppositePlayer = alivePiece[0].color != alivePiece[1].color;
        bool isSameTile = alivePiece[0].tileColor == alivePiece[1].tileColor;
        if (isBothBishop && isOppositePlayer && isSameTile)
            return STATE_FORCED_DRAW_INSUFFICIENT_MATERIAL;
    }
    // else normal
    return STATE_NORMAL;
}

auto Board::hasZeroMoves() -> bool {
    auto moves = getMoves(1);
    return moves.empty();
}

auto Board::hasOneMove() -> bool {
    auto moves = getMoves(2);
    return moves.size() == 1;
}

auto Board::getMoves(u8 count) -> std::vector<Move> {
    const u8 mycolor = whiteMove() ? PIECE_WHITE : PIECE_BLACK;
    std::vector<Move> moves;
    moves.reserve(64);

    // Loop over all squares and find our pieces.
    for (u8 row = 0; row < GRID_LENGTH; ++row)
        for (u8 column = 0; column < GRID_LENGTH; ++column) {
            // Adds a move but only if it is legal.
            // Required because for loops below will try to add lots
            // of out-of-bounds and weird, illegal moves.
            auto tryAddMove = [&column, &row, &moves, this](u8 toCol, u8 toRow,
                                                            u8 promote = 0U) {
                const Move move{column, toCol, row, toRow, promote};
                if (isMoveLegal(move))
                    moves.push_back(move);
            };

            const auto walkRay = [&](int dc, int dr) {
                if (dc == 0 && dr == 0)
                    return;
                int col = column + dc;
                int rw = row + dr;
                while (inBounds(col, rw)) {
                    const auto piece = pieceAt(col, rw);
                    if (piece != EMPTY && (piece & COLOR_MASK) == WhoseTurn())
                        break; // blocked by own piece
                    const auto move = Move{column, static_cast<u8>(col), row, static_cast<u8>(rw), 0};
                    if (!isMoveIntoCheck(move))
                        moves.push_back(move);
                    if (piece != EMPTY)
                        break; // capture, can't go any further
                    col += dc;
                    rw += dr;
                }
            };

            const u8 piece = pieceAt(column, row);
            if ((piece & COLOR_MASK) == mycolor) {
                switch (piece & TYPE_MASK) {
                case PIECE_BISHOP: {
                    // Check all diagonal vectors
                    for (int dc : {-1, 1})
                        for (int dr : {-1, 1})
                            walkRay(dc, dr);
                    break;
                }
                case PIECE_KING: {
                    // For King, we just need to try and
                    // move to all 8 surrounding tiles
                    for (int c : {-1, 0, 1})
                        for (int r : {-1, 0, 1})
                            tryAddMove(column + c, row + r);
                    // Try adding castling moves
                    tryAddMove(column - 2, row);
                    tryAddMove(column + 2, row);
                    break;
                }
                case PIECE_KNIGHT: {
                    // For knight, try all pieces that are
                    // delta row = 2 and delta column = 1 or
                    // delta row = 1 and delta column = 2.
                    for (int c : {-2, -1, 1, 2})
                        for (int r : {-2, -1, 1, 2})
                            if (std::abs(c) != std::abs(r))
                                tryAddMove(column + c, row + r);
                    break;
                }
                case PIECE_PAWN: {
                    // All possible pawn moves; +1 row, +2
                    // rows, +1 row +1 column (for capture).
                    u8 color = piece & COLOR_MASK;
                    for (int dc : {-1, 0, 1})
                        for (int dr : {1, 2}) {
                            if (dc && dr == 2)
                                continue;

                            // Check for promotion
                            const u8 newRow = (color == PIECE_WHITE) ? row + dr : row - dr;
                            const bool isPromote = (newRow == GRID_LENGTH - 1) || newRow == 0;
                            if (isPromote) {
                                for (u8 type : {PIECE_QUEEN, PIECE_KNIGHT, PIECE_ROOK, PIECE_BISHOP})
                                    tryAddMove(column + dc, newRow, type);
                            } else {
                                tryAddMove(column + dc, newRow);
                            }
                        }
                    break;
                }
                case PIECE_QUEEN: {
                    // Queen, we need to check all diagnoal
                    // vectors + the horizontal and vertical
                    // axis
                    walkRay(-1, -1);
                    walkRay(-1, 0);
                    walkRay(-1, 1);
                    walkRay(0, -1);
                    walkRay(0, 1);
                    walkRay(1, -1);
                    walkRay(1, 0);
                    walkRay(1, 1);
                    break;
                }
                case PIECE_ROOK:
                case PIECE_CASTLE: {
                    walkRay(-1, 0);
                    walkRay(1, 0);
                    walkRay(0, -1);
                    walkRay(0, 1);
                    break;
                }
                }
                // Break out if we've hit the move limit.
                if (count && moves.size() >= count) {
                    return moves;
                }
            }
        }
    return moves;
}

auto Board::isCheck(u8 color) -> bool {
    const u8 idx = (color >> 3) * 2;
    return isAttacked(m_kingPos[idx], m_kingPos[idx + 1], PIECE_KING | color);
}

auto Board::whiteMove() const -> bool { return !(m_bits & BLACKMOVE_MASK); }

auto Board::enPassantColumn() const -> u8 {
    return (m_bits & DOUBLE_MASK) ? (m_bits & PAWN_MASK) >> 2 : UINT8_MAX;
}

auto Board::canCastle(u8 color, bool queenSide) -> bool {
    if (color == PIECE_WHITE && !queenSide)
        return pieceAt(7, 0) == (PIECE_CASTLE | color);
    else if (color == PIECE_WHITE && queenSide)
        return pieceAt(0, 0) == (PIECE_CASTLE | color);
    else if (color == PIECE_BLACK && !queenSide)
        return pieceAt(7, 7) == (PIECE_CASTLE | color);
    else if (color == PIECE_BLACK && queenSide)
        return pieceAt(0, 7) == (PIECE_CASTLE | color);
    else
        return false;
}

auto Board::isStale() -> bool { return m_bits & STALE_MASK; }

auto Board::isAttacked(u8 column, u8 row) const -> bool {
    const u8 piece = pieceAt(column, row);
    if (!piece)
        return false;
    return isAttacked(column, row, piece);
}

auto Board::isAttacked(u8 column, u8 row, u8 piece) const -> bool {
    const u8 attackerColor = (piece & COLOR_MASK) == PIECE_WHITE ? PIECE_BLACK : PIECE_WHITE;
    // First of all, check knight moves
    // All squares around the tile that are +2 rows, +2 colums. (or -ve)
    const u8 knight = (PIECE_KNIGHT | attackerColor);
    for (int r : {-1, 1})
        for (int c : {-1, 1}) {
            if (inBounds(column + c * 2, row + r) &&
                pieceAt(column + c * 2, row + r) == knight)
                return true;
            else if (inBounds(column + c, row + r * 2) &&
                     pieceAt(column + c, row + r * 2) == knight)
                return true;
        }

    for (int colMove : {-1, 0, 1})
        for (int rowMove : {-1, 0, 1}) {
            // Skip no move
            if (!colMove && !rowMove)
                continue;

            // Iterate until we find a piece
            bool foundPiece = false;

            // Check all tiles on this vector until we find piece,
            // or leave confines of the grid.
            for (u8 c = column + colMove, r = row + rowMove;
                 c < GRID_LENGTH && r < GRID_LENGTH && !foundPiece;
                 c += colMove, r += rowMove) {
                const u8 attackerPiece = pieceAt(c, r);
                if (!attackerPiece)
                    continue; // Move to next tile, nothing
                              // of interest here

                foundPiece = true;
                if ((attackerPiece & COLOR_MASK) != attackerColor)
                    break; // Blocked by our own piece, so
                           // ignore this vector

                const u8 dcol = abs(c - column);
                const u8 drow = abs(r - row);
                switch (attackerPiece & TYPE_MASK) {
                case PIECE_BISHOP:
                    if (dcol && drow && dcol == drow)
                        return true;
                    break;
                case PIECE_KING:
                    if (dcol <= 1 && drow <= 1)
                        return true;
                    break;
                case PIECE_PAWN:
                    if (dcol == 1 &&
                        rowMove ==
                            ((attackerColor & COLOR_MASK) == PIECE_WHITE ? -1 : 1))
                        return true;
                    break;
                case PIECE_QUEEN:
                    return true;
                case PIECE_ROOK:
                case PIECE_CASTLE:
                    if ((dcol && !drow) || (!dcol && drow))
                        return true;
                    break;
                }
            }
        }
    return false;
}

Undo Board::MakeNewMove(const Move &move) {
    Undo u;
    u.nChanges = 0;
    u.bits = m_bits;
    u.king = m_kingPos;

    auto record = [&](u8 col, u8 row) {
        const u8 idx = col * GRID_LENGTH + row;
        u.changes[u.nChanges++] = {idx, m_pieces[idx]};
    };

    auto src = pieceAt(move.fromCol, move.fromRow);
    const auto mycolor = WhoseTurn();
    const auto typ = src & TYPE_MASK;

    const auto isCapture = pieceAt(move.toCol, move.toRow) != EMPTY;
    const auto isEP = (src & TYPE_MASK) == PIECE_PAWN && isEnPassant(move);
    const auto isCastling = move.fromCol == 4 && (move.toCol == 6 || move.toCol == 2);

    m_bits &= ~(DOUBLE_MASK | PAWN_MASK);
    src = typ == PIECE_CASTLE ? (PIECE_ROOK | mycolor) : src;

    if (typ == PIECE_KING) {
        if (isCastling) {
            const auto kingSide = move.toCol == 6;
            const auto rFrom = kingSide ? 7 : 0;
            const auto rTo = kingSide ? 5 : 3;
            record(rFrom, move.fromRow);
            record(rTo, move.fromRow);
            removePiece(rFrom, move.fromRow);
            setPiece(PIECE_ROOK | mycolor, rTo, move.fromRow);
        } else {
            for (auto col : {0, 7}) {
                if (pieceAt(col, move.fromRow) == (PIECE_CASTLE | mycolor)) {
                    record(col, move.fromRow);
                    setPiece(PIECE_ROOK | mycolor, col, move.fromRow);
                }
            }
        }
    }

    if (typ == PIECE_PAWN) {
        if (abs(move.fromRow - move.toRow) == 2) {
            m_bits |= DOUBLE_MASK;
            m_bits |= (move.fromCol << 2);
        }
        if (move.promotion) {
            src = (move.promotion & TYPE_MASK) | mycolor;
        }
    }

    if (typ == PIECE_PAWN || isCapture || isEP) {
        m_bits &= ~STALE_MASK;
    } else {
        m_bits |= STALE_MASK;
    }

    record(move.toCol, move.toRow);
    record(move.fromCol, move.fromRow);
    setPiece(src, move.toCol, move.toRow);
    removePiece(move.fromCol, move.fromRow);
    if (isEP) {
        record(move.toCol, move.fromRow);
        removePiece(move.toCol, move.fromRow);
    }

    m_bits ^= BLACKMOVE_MASK;
    return u;
}

void Board::UndoMove(const Undo &undo) {
    for (int i = 0; i < undo.nChanges; i++) {
        m_pieces[undo.changes[i].index] = undo.changes[i].value;
    }
    m_bits = undo.bits;
    m_kingPos = undo.king;
}
