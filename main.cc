#include "Bitboard.h"
#include "CLI11.hpp"
#include "FEN.h"
#include "Perft.h"
#include "Player.h"
#include "Test.h"
#include "Viewer.h"
#include <algorithm>
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
    Bench,
    List,
    Perft,
    Test,
    Watch,
};

struct Args {
    Mode mode;
    int depth = 0;
    std::vector<std::string> players;
    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
};

static Args parseArgs(int argc, char **argv)
{
    auto args = Args{};
    auto app = CLI::App{"chessview"};
    app.require_subcommand(1);
    app.set_help_all_flag("--help-all", "Show help for all subcommands");

    auto *bench = app.add_subcommand("bench", "Benchmark random games");
    bench->add_option("--count", args.depth, "Number of games to run")->default_val(1000);
    bench->callback([&] { args.mode = Mode::Bench; });

    app.add_subcommand("list", "List available players")->callback([&] { args.mode = Mode::List; });

    auto *perft = app.add_subcommand("perft", "Run a perft check");
    perft->add_option("--depth", args.depth, "Search depth")->default_val(5);
    perft->add_option("--fen", args.fen, "Starting position (FEN)");
    perft->callback([&] { args.mode = Mode::Perft; });

    app.add_subcommand("test", "Run perft accuracy tests")->callback([&] { args.mode = Mode::Test; });

    auto *watch = app.add_subcommand("watch", "Watch two engines play against each other");
    watch->add_option("players", args.players, "Players")->required()->expected(2);
    watch->add_option("--fen", args.fen, "Starting position (FEN)");
    watch->callback([&] { args.mode = Mode::Watch; });

    try {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError &e) {
        std::exit(app.exit(e));
    }

    return args;
}

void PlayerList()
{
    for (const auto &player : GetPlayerList()) {
        std::cout << player.name << ": " << player.description << '\n';
    }
}

void DoPerft(int depth, std::string fen)
{
    auto p = ParseFEN(fen);

    const auto getTile = [&](int index) {
        std::string s = "";
        s += "abcdefgh"[index % 8];
        s += "12345678"[index / 8];
        return s;
    };

    const auto moves = GenerateMoves(p);
    auto tot = uint64_t(0);

    for (const auto &move : moves) {
        const auto undo = MakeMove(p, move);
        const auto r = Perft(p, depth - 1);
        tot += r;
        UndoMove(p, move, undo);
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
        for (;;) {
            const auto state = GetPositionState(p);
            if (state != S_NRM)
                break;

            const auto moves = GenerateMoves(p);
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
        .fen = opt.fen,
    };
    RunViewer(viewOptions);
}

int main(int argc, char **argv)
{
    programName = argv[0];

    InitLookupTables();

    const auto opt = parseArgs(argc, argv);
    switch (opt.mode) {
    case Mode::Bench:
        Bench(opt.depth);
        break;
    case Mode::List:
        PlayerList();
        break;
    case Mode::Perft:
        DoPerft(opt.depth, opt.fen);
        break;
    case Mode::Test:
        RunTests();
        break;
    case Mode::Watch:
        Watch(opt);
        break;
    }
}
