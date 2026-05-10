#include "FEN.h"
#include "Bitboard.h"
#include <cctype>
#include <sstream>
#include <unordered_map>

using std::unordered_map;

static const unordered_map<char, PieceType> PIECE_MAP = {
    {'p', PAWN},
    {'k', KING},
    {'q', QUEEN},
    {'b', BISHOP},
    {'n', KNIGHT},
    {'r', ROOK},
};

static const unordered_map<char, Castling> CASTLING_BIT_MAP = {
    {'K', CR_WK},
    {'Q', CR_WQ},
    {'k', CR_BK},
    {'q', CR_BQ},
};

static Piece PieceFromChar(char c)
{
    if (!PIECE_MAP.contains(std::tolower(static_cast<unsigned char>(c)))) {
        return NONE;
    }
    const auto color = std::isupper(static_cast<unsigned char>(c)) ? CWHITE : CBLACK;
    const auto piece = PIECE_MAP.at(std::tolower(static_cast<unsigned char>(c)));
    return MakePiece(color, piece);
}

Position ParseFEN(const std::string &str)
{
    auto p = Position{};

    auto iss = std::istringstream(str);
    auto placement = std::string();
    auto side = std::string();
    auto castling = std::string();
    auto ep = std::string();
    auto half = 0;
    auto full = 1;
    iss >> placement >> side >> castling >> ep >> half >> full;

    auto row = 7;
    auto column = 0;
    for (const auto &c : placement) {
        if (c == '/') {
            row--;
            column = 0;
        }
        else if (std::isdigit(static_cast<unsigned char>(c))) {
            column += c - '0';
        }
        else {
            const auto pc = PieceFromChar(c);
            const auto color = ColorOf(pc);
            const auto type = TypeOf(pc);
            if (pc != NONE) {
                p.bitboards[color][type] |= (1ULL << (row * 8 + column));
                p.occupancy[color] |= (1ULL << (row * 8 + column));
            }
            column += 1;
        }
    }

    p.whoseturn = (side == "w" ? CWHITE : CBLACK);
    p.castling = 0;
    for (const auto &[key, value] : CASTLING_BIT_MAP) {
        if (castling.find(key) != std::string::npos) {
            p.castling |= value;
        }
    }

    if (ep == "-" || ep.size() != 2) {
        p.epsq = NO_EP;
    }
    else {
        p.epsq = (ep[1] - '1') * 8 + (ep[0] - 'a');
    }

    p.half = half;
    return p;
}
