# chessview

**chessview** is an application for interacting with Chess bots and making it easy to add your own. Its main purpose is to watch your own bots play against each other, although it is a full-fledged chess engine in itself.

It was originally written sometime in 2020.

![Application preview image](Img/preview.png)

## Building

Install required dependencies:

- g++
- SDL2
- SDL2_image

Then run `make` in the root directory. It will generate a `chess` binary.

## Usage

### Command line

#### Perft

Perft mode runs a [Perft](https://www.chessprogramming.org/Perft) check using the built-in board representation to a depth of 6.

```
$ chessview --perft
Perft(1): 20 0ms
Perft(2): 400 0ms
Perft(3): 8902 1ms
...
```

#### Player list

To obtain a list of available bots:

`chessview --players`

```
$ chessview --players
blacksquares
bongcloud
centre
...
```

#### Running a bot game

To run a game, specify which bots to play for both white and black sides:

```
$ chessview white black
...
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

## Features

- Implements the more complex rules of chess, including **en passant**, **castling** and **most stalemate conditions** (with the exception of three-fold repetition).
- Generates all legal moves for a position.
- Ability to easily add additional bots into the application.

## Planned

- Many more dumb but interesting bots.
- Threefold repetition stalemate [rule](https://en.wikipedia.org/wiki/Threefold_repetition).
- Support for PGN string imports (to watch existing games).
- FEN import, to start from a known position.
- Tournaments between bots.
- Recording of games to files (probably in PGN notation).
