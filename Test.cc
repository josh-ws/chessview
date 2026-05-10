#include "Test.h"
#include "Bitboard.h"
#include "FEN.h"
#include "Perft.h"
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

struct Test {
    std::string fen;
    int depth;
    uint64_t exp;
};

static std::vector<Test> cases = {
    Test{
        .fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        .depth = 6,
        .exp = 119060324ULL,
    },
    Test{
        .fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        .depth = 5,
        .exp = 193690690ULL,
    },
    Test{
        .fen = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
        .depth = 6,
        .exp = 11030083ULL,
    },
    Test{
        .fen = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        .depth = 5,
        .exp = 15833292ULL,
    },
    Test{
        .fen = "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1",
        .depth = 5,
        .exp = 15833292ULL,
    },
    Test{
        .fen = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
        .depth = 5,
        .exp = 89941194ULL,
    },
    Test{
        .fen = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
        .depth = 5,
        .exp = 164075551ULL,
    },
};

void RunTests()
{
    auto ok = true;
    for (const auto &test : cases) {
        auto p = ParseFEN(test.fen);
        auto actual = Perft(p, test.depth);
        auto result = test.exp == actual ? "OK" : "FAIL";
        if (test.exp != actual) {
            ok = false;
        }
        std::cout << "TEST(" << test.fen << "): " << result << "\n";
    }
    if (!ok) {
        std::cout << "One or more tests failed\n";
    }
}
