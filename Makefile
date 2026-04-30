CC=clang++
CFLAGS=-O3  -march=native -pipe -Wall -Wextra -Wfatal-errors -pedantic -Weffc++ -pedantic-errors --std=c++23
DEPENDS=-lraylib
OUT=chess

all: Bitboard.cc Viewer.cc main.cc Player.cc FEN.cc Perft.cc
	$(CC) $(DEPENDS) $(CFLAGS) Bitboard.cc Viewer.cc main.cc Player.cc FEN.cc Perft.cc -o $(OUT)
