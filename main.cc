#include "Bitboard.h"
#include "Perft.h"
#include "Player.h"
#include "Viewer.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <random>
#include <string>
#include <vector>

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::steady_clock;

constexpr int PerftMaxDepth = 6;
constexpr int BenchGames = 100000;

enum class Mode {
    Unknown = -1,
    Help,
    Bench,
    List,
    Perft,
    Play,
    Watch,
};

const static std::string modes[] = {"help", "bench", "list", "perft", "play", "watch"};

struct Args {
    Mode mode = Mode::Unknown;
    std::vector<std::string> players;
};

static Args parseArgs(int argc, char **argv)
{
    auto vec = std::vector<std::string>{argv, argv + argc};
    auto args = Args{};
    if (vec.size() <= 1) {
        args.mode = Mode::Help;
        return args;
    }
    for (int i = 0; i < 6; i++) {
        if (vec[1] == modes[i]) {
            args.mode = static_cast<Mode>(i);
        }
    }
    std::copy_if(vec.begin() + 1, vec.end(), std::back_inserter(args.players),
                 [argv](const auto &arg) { return arg != argv[0] && arg[0] != '-'; });
    return args;
}

[[noreturn]] void UsageAndExit(std::string progName, std::string msg = "")
{
    if (!msg.empty()) {
        std::cerr << msg << '\n';
    }
    std::cerr << "Usage: " << '\n';
    std::cerr << "    " << progName << " bench                  Benchmark running some games\n";
    std::cerr << "    " << progName << " list                   Returns a list of available players\n";
    std::cerr << "    " << progName << " perft                  Run a perft performance/accuracy check\n";
    std::cerr << "    " << progName << " play [player]          Play against the specified bot\n";
    std::cerr << "    " << progName << " watch [white] [black]  Watch `white` play `black`\n";
    std::exit(1);
}

void PlayerList()
{
    for (const auto &player : GetPlayerList()) {
        std::cout << player.name << ": " << player.description << '\n';
    }
}

void DoPerft()
{
    static std::array<Move, MAX_MOVES> moves;

    const auto getTile = [&](int index) {
        std::string s = "";
        s += "abcdefgh"[index % 8];
        s += "12345678"[index / 8];
        return s;
    };

    auto p = CreateDefaultPosition();
    auto n = GenerateMoves(p, moves);
    auto tot = uint64_t(0);

    for (int i = 0; i < n; i++) {
        const auto &move = moves[i];
        auto newPos = p;
        MakeMove(newPos, move);

        const auto r = Perft(newPos, PerftMaxDepth - 1);
        tot += r;
        std::cout << getTile(move.from) << getTile(move.to) << ": " << r << "\n";
    }

    std::cout << "\nNodes searched: " << tot << "\n";
}

void Bench()
{
    auto t1 = steady_clock::now();
    std::cout << "Running " << BenchGames << " games..." << std::endl;

    std::array<Move, MAX_MOVES> moves;
    std::mt19937 rng{std::random_device{}()};

    for (int i = 0; i < BenchGames; i++) {
        auto board = CreateDefaultPosition();
        for (int i = 0; i < 60; i++) {
            const auto n = GenerateMoves(board, moves);
            if (!n) {
                break;
            }
            std::uniform_int_distribution<int> dist(0, n - 1);
            MakeMove(board, moves[dist(rng)]);
        }
    }
    auto t2 = steady_clock::now();
    std::cout << "Done in " << duration_cast<milliseconds>(t2 - t1).count() << "ms" << "\n";
}

void Watch(const Args &opt)
{
    const auto viewOptions = ViewerOptions{
        .title = "chessview",
        .width = 480,
        .height = 480,
        .players = opt.players,
    };
    RunViewer(viewOptions);
}

int main(int argc, char **argv)
{
    InitLookupTables();

    const auto opt = parseArgs(argc, argv);
    switch (opt.mode) {
    case Mode::Unknown:
        UsageAndExit(argv[0], "Unrecognized command");
        break;
    case Mode::Help:
        UsageAndExit(argv[0]);
        break;
    case Mode::Bench:
        Bench();
        break;
    case Mode::List:
        PlayerList();
        break;
    case Mode::Perft:
        DoPerft();
        break;
    case Mode::Play:
        std::cout << "not implemented!\n";
        break;
    case Mode::Watch:
        Watch(opt);
        break;
    }
}
