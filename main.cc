#include "Board.h"
#include "Perft.h"
#include "Player.h"
#include "Runner.h"
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <iterator>

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::steady_clock;

using PlayerCreator = std::function<std::unique_ptr<Player>()>;

// List of available players
static const auto playerCreators = std::map<std::string, PlayerCreator>{
    {"random", makeRandom},
    {"whitesquares", makeWhiteSquares},
    {"blacksquares", makeBlackSquares},
    {"min", makeMinimizeOpponentMoves},
    {"max", makeMaximizeOpponentMoves},
    {"min_self", makeMinimizeOwnMoves},
    {"max_self", makeMaximizeOwnMoves},
    {"defensive", makeDefensive},
    {"offensive", makeOffensive},
    {"suicidal", makeSuicidal},
    {"pacifist", makePacifist},
    {"edge", makeClearPath},
    {"centre", makeCentre},
    {"bongcloud", makeBongCloud},
};

struct Args {
    bool isPerft;
    bool isHeadless;
    std::vector<std::string> players;
};

static Args parseArgs(int argc, char **argv) {
    auto args = Args{};
    auto vec = std::vector<std::string>{argv, argv + argc};
    args.isHeadless =
        std::find(vec.begin(), vec.end(), "-headless") != vec.end();
    args.isPerft = std::find(vec.begin(), vec.end(), "-perft") != vec.end();

    std::copy_if(
        vec.begin(), vec.end(), std::back_inserter(args.players),
        [argv](const auto &arg) { return arg != argv[0] && arg[0] != '-'; });

    return args;
}

static auto makePlayers(std::vector<std::string> playerNames) {
    auto players = std::vector<std::unique_ptr<Player>>{};
    std::transform(playerNames.begin(), playerNames.end(),
                   std::back_inserter(players), [](const std::string &name) {
                       if (playerCreators.contains(name)) {
                           return playerCreators.at(name)();
                       }
                       return makeRandom();
                   });
    return players;
}

static auto runHeadless(std::vector<std::unique_ptr<Player>> &&players) {
    auto runner = RunnerStd{std::move(players[0]), std::move(players[1])};
    const auto result = runner.run();
    std::cout << result << std::endl;
}

static auto runViewer(std::vector<std::unique_ptr<Player>> &&players) {
    auto runner = RunnerUI{};
    runner.addPlayer(BLACK, std::move(players.back()));
    if (players.size() == 2) { // either player or computer can play as white
        runner.addPlayer(WHITE, std::move(players.front()));
    }

    const auto result = runner.run();
    std::cout << result << std::endl;
}

int main(int argc, char **argv) {
    const auto args = parseArgs(argc, argv);

    if (args.isPerft) {
        constexpr int PerftMaxDepth = 6;
        for (int i = 1; i <= PerftMaxDepth; i++) {
            auto board = CreateDefaultBoard();
            auto t1 = steady_clock::now();
            auto result = Perft(board, i);
            auto t2 = steady_clock::now();
            auto ms = duration_cast<milliseconds>(t2 - t1).count();
            std::cout << "Perft(" << i << "): " << result << " " << ms << "ms"
                      << "\n";
        }
        return EXIT_SUCCESS;
    }

    if (args.players.size() == 0 ||
        (args.isHeadless && args.players.size() != 2)) {
        std::cerr << "Invalid configuration"
                  << "\n"
                  << "Usage: " << argv[0] << " [-headless] [white] black"
                  << std::endl;
        std::exit(1);
    }

    for (const auto &name : args.players) {
        if (!playerCreators.contains(name)) {
            std::cerr << "Unrecognized player: " << name
                      << " (assuming `random`)" << std::endl;
        }
    }

    if (args.isHeadless) {
        runHeadless(makePlayers(args.players));
    } else {
        runViewer(makePlayers(args.players));
    }
}
