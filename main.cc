#include "Bitboard.h"
#include "FEN.h"
#include "Perft.h"
#include "Player.h"
#include "Types.h"
#include "Viewer.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <random>
#include <string>

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::steady_clock;

constexpr int PerftMaxDepth = 6;
constexpr int ExpectedViewArguments = 2;
constexpr int BenchGames = 100000;

struct Args {
    bool isHelp;
    bool isPlayerList;
    bool isPerft;
    bool isHeadless;
    bool isBench;
    std::vector<std::string> players;
};

static Args parseArgs(int argc, char **argv) {
    auto args = Args{};
    auto vec = std::vector<std::string>{argv, argv + argc};
    args.isHelp = std::find(vec.begin(), vec.end(), "--help") != vec.end() || std::find(vec.begin(), vec.end(), "-h") != vec.end();
    args.isPlayerList = std::find(vec.begin(), vec.end(), "--players") != vec.end();
    args.isHeadless = std::find(vec.begin(), vec.end(), "--headless") != vec.end();
    args.isPerft = std::find(vec.begin(), vec.end(), "--perft") != vec.end();
    args.isBench = std::find(vec.begin(), vec.end(), "--bench") != vec.end();
    std::copy_if(vec.begin(), vec.end(), std::back_inserter(args.players),
                 [argv](const auto &arg) { return arg != argv[0] && arg[0] != '-'; });

    return args;
}

[[noreturn]] void UsageAndExit(std::string progName, std::string msg = "") {
    if (!msg.empty()) {
        std::cerr << msg << '\n';
    }
    std::cerr << "Usage: " << '\n';
    std::cerr << "    " << progName << " --perft      Run a perft performance/accuracy check (may take some time)" << '\n';
    std::cerr << "    " << progName << " --players    Output a list of players" << '\n';
    std::cerr << "    " << progName << " white black  View a game between `white` and `black`" << '\n';
    std::exit(1);
}

int main(int argc, char **argv) {
    InitLookupTables();

    const auto args = parseArgs(argc, argv);

    if (args.isHelp) {
        UsageAndExit(argv[0]);
    }

    if (args.isPlayerList) {
        for (const auto &player : GetPlayerList()) {
            std::cout << player.name << ": " << player.description << '\n';
        }
        return EXIT_SUCCESS;
    }

    if (args.isPerft) {
        for (int i = 1; i <= PerftMaxDepth; i++) {
            auto board = CreateDefaultPosition();
            auto t1 = steady_clock::now();
            auto result = Perft(board, i);
            auto t2 = steady_clock::now();
            std::cout << "Perft(" << i << "): " << result << " " << duration_cast<milliseconds>(t2 - t1).count() << "ms" << "\n";
        }
        return EXIT_SUCCESS;
    }

    if (args.isBench) {
        auto t1 = steady_clock::now();
        std::cout << "Running " << BenchGames << " games..." << std::endl;

        std::array<Move, MAX_MOVES> moves;
        std::mt19937 rng{std::random_device{}()};

        for (int i = 0; i < 1; i++) {
            auto board = CreateDefaultPosition();
            auto stale = 0;

            for (int i = 0; i < 600; i++) {
                const auto n = GenerateMoves(board, moves);
                if (!n) {
                    break;
                }
                std::cout << "moves: " << n << " " << ToFEN(board) << "\n";

                std::uniform_int_distribution<int> dist(0, n - 1);
                MakeMove(board, moves[dist(rng)]);

                // if (board.getBoardState(stale) != STATE_NORMAL) {
                //     break;
                // }
            }
        }
        auto t2 = steady_clock::now();
        std::cout << "Done in " << duration_cast<milliseconds>(t2 - t1).count() << "ms" << "\n";
        return EXIT_SUCCESS;
    }

    if (args.players.size() != ExpectedViewArguments) {
        UsageAndExit(argv[0], "invalid configuration");
    }

    for (int i = 0; i < args.players.size(); i += 1) {
        std::cout << "player[" << i << "]: " << args.players[i] << '\n';
    }

    const auto viewOptions = ViewerOptions{
        .title = "chessview",
        .width = 480,
        .height = 480,
        .players = args.players,
    };
    RunViewer(viewOptions);
}
