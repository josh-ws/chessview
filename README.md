# chessview

**chessview** is an application for interacting with Chess bots and making it easy to add your own. Its main purpose is to watch your own bots play against each other, although it is a full-fledged chess engine in itself.

It was originally written sometime in 2020.

![Application preview image](Img/preview.png)

## Features

- Over a dozen built-in weird Chess engines.
- Trivially easy to extend with new bots by adding one to `Player.cc`.
- Full legal move generation including en passant, castling and draw conditions verified against [perft results](https://www.chessprogramming.org/Perft_Results) (via `chessview test`).
- Fast implementation using Bitboards. Perft(6) in ~1.8s and can play around 5,000 games per second.

## Building

Install required dependencies:

- C++ compiler (g++, clang, ...)
- raylib

Then run `make` in the root directory. It will generate a `chess` binary.

## Usage

```
chessview <command> [options]
```

Run `chessview --help-all` for a full reference of every command. Each command also accepts `--help` individually (e.g. `chessview perft --help`).

### Commands

#### `bench`: benchmark random games

Plays N random games and reports elapsed time. Defaults to 1000 games.

```
$ chessview bench --count 1000
Running 1000 games...
Done in 312ms
```

#### `perft`: perft accuracy/performance check

Runs a [Perft](https://www.chessprogramming.org/Perft) check from a given position. Defaults to depth 5 from the standard starting position.

```
$ chessview perft --depth 5
$ chessview perft --depth 4 --fen "<FEN string>"
```

#### `watch`: watch two bots play

Specify the two bots playing white and black:

```
$ chessview watch random center
$ chessview watch random center --fen "<FEN string>"
```

Viewer controls:

- **Space**: pause the viewer. No new moves will be executed. Press again to unpause.

#### `list`: list available bots

```
$ chessview list
blacksquares: ...
center: ...
...
```

#### `test`: run perft against a suite of known positions

Verifies move generation correctness against a set of well-known test positions.

```
$ chessview test
```

### Players

These are the built-in bots, ready to play against.

- random: Plays completely random moves using mt19937
- whitesquares: Plays moves that land pieces on white tiles
- blacksquares: Plays moves that land pieces on black tiles
- center: Moves pieces as close to the center as possible
- min_oppt: Tries to minimize the number of moves the opponent has
- max_oppt: Tries to maximize the number of moves the opponent has
- min_self: Tries to minimize the number of moves that the player has
- max_self: Tries to maximize the number of moves that the player has
- defensive: Tries to minimize the number of own pieces under attack
- offensive: Tries to maximize the number of enemy pieces under attack
- reckless: Tries to maximize the number of own pieces under attack
- pacifist: Tries to minumize the number of enemy pieces under attack
- edge: Tries to move pieces towards the edge of the board
- aggressive: Tries to move pieces towards the enemy's side
- passive: Tries not to move pieces towards the enemy's side

## Planned

- Many more dumb but interesting bots.
- Threefold repetition stalemate [rule](https://en.wikipedia.org/wiki/Threefold_repetition).
- Support for PGN string imports (to watch existing games).
- Tournaments between bots.
- Recording of games to files (probably in PGN notation).

## Licensing

This project is MIT licensed (see LICENSE) and bundles:

- [CLI11](https://github.com/CLIUtils/CLI11) - Command line parsing, BSD 3-clause
