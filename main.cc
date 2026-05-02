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
#include <stdexcept>
#include <string>
#include <vector>

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::steady_clock;

static std::string programName = "";

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

void Die(std::string msg = "")
{
    if (!msg.empty()) {
        std::cerr << msg << '\n';
    }
    std::cerr << "Usage: " << '\n';
    std::cerr << "    " << programName << " bench [games]          Benchmark the time it takes to run provided number of games\n";
    std::cerr << "    " << programName << " list                   Returns a list of available players\n";
    std::cerr << "    " << programName << " perft [depth]          Run a perft performance/accuracy check with the specified depth\n";
    std::cerr << "    " << programName << " watch [white] [black]  Watch `white` play `black`\n";
    std::exit(1);
}

struct Args {
    Mode mode = Mode::Unknown;
    int depth;
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
    std::copy_if(vec.begin() + 2, vec.end(), std::back_inserter(args.players),
                 [argv](const auto &arg) { return arg != argv[0] && arg[0] != '-'; });

    if (args.mode == Mode::Perft || args.mode == Mode::Bench) {
        if (vec.size() >= 3) {
            args.depth = std::stoi(vec.back());
        }
        else if (args.mode == Mode::Perft) {
            args.depth = 5;
        }
        else if (args.mode == Mode::Bench) {
            args.depth = 10'000;
        }
    }

    return args;
}

void PlayerList()
{
    for (const auto &player : GetPlayerList()) {
        std::cout << player.name << ": " << player.description << '\n';
    }
}

void DoPerft(int depth)
{
    const auto getTile = [&](int index) {
        std::string s = "";
        s += "abcdefgh"[index % 8];
        s += "12345678"[index / 8];
        return s;
    };

    auto p = CreateDefaultPosition();
    const auto moves = GenerateMoves(p);
    auto tot = uint64_t(0);

    for (const auto &move : moves) {
        auto newPos = p;
        MakeMove(newPos, move);

        const auto r = Perft(newPos, depth - 1);
        tot += r;
        std::cout << getTile(move.from) << getTile(move.to) << ": " << r << "\n";
    }

    std::cout << "\nNodes searched: " << tot << "\n";
}

void Bench(int depth)
{
    auto t1 = steady_clock::now();
    std::cout << "Running " << depth << " games..." << std::endl;

    std::mt19937 rng{std::random_device{}()};

    for (int i = 0; i < depth; i++) {
        auto p = CreateDefaultPosition();
        for (int i = 0; i < 60; i++) { // TODO
            const auto moves = GenerateMoves(p);
            if (moves.size() == 0) {
                break;
            }
            std::uniform_int_distribution<int> dist(0, moves.size() - 1);
            MakeMove(p, moves[dist(rng)]);
        }
    }
    auto t2 = steady_clock::now();
    std::cout << "Done in " << duration_cast<milliseconds>(t2 - t1).count() << "ms" << "\n";
}

void Watch(const Args &opt)
{
    if (opt.players.size() != 2)
        throw std::runtime_error("need 2 players to watch");

    const auto viewOptions = ViewerOptions{
        .title = "chessview",
        .width = 480,
        .height = 480,
        .players = {opt.players},
    };
    RunViewer(viewOptions);
}

int main(int argc, char **argv)
{
    programName = argv[0];

    InitLookupTables();

    const auto opt = parseArgs(argc, argv);
    switch (opt.mode) {
    case Mode::Unknown:
        Die("Unrecognized command");
        break;
    case Mode::Help:
        Die();
        break;
    case Mode::Bench:
        Bench(opt.depth);
        break;
    case Mode::List:
        PlayerList();
        break;
    case Mode::Perft:
        DoPerft(opt.depth);
        break;
    case Mode::Play:
        Die("Not implemented");
        break;
    case Mode::Watch:
        Watch(opt);
        break;
    }
}
