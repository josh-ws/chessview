CC=g++
CFLAGS=-Ofast -march=native -pipe -Wall -Wextra -Wfatal-errors -pedantic -Weffc++ -pedantic-errors --std=c++20
DEPENDS=-lSDL3 -lSDL3_image
OUT=chess

all: Board.cc Viewer.cc Runner.cc main.cc Player.cc RunnerStd.cc RunnerUI.cc FEN.cc Perft.cc
	$(CC) $(DEPENDS) $(CFLAGS) Board.cc Viewer.cc Runner.cc main.cc Player.cc RunnerStd.cc RunnerUI.cc FEN.cc Perft.cc -o $(OUT)
