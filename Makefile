CC=clang++
CFLAGS=-O3 -march=native -pipe -Wall -Wextra -Wfatal-errors -pedantic -Weffc++ -pedantic-errors --std=c++23 -MMD -MP
LDLIBS=-lraylib
OUT=chess
BUILD=build

SRCS = Bitboard.cc Viewer.cc main.cc Player.cc FEN.cc Perft.cc Test.cc
OBJS = $(SRCS:%.cc=$(BUILD)/%.o)
DEPS = $(OBJS:.o=.d)

all: $(OUT)

$(OUT): $(OBJS)
	$(CC) $(OBJS) $(LDLIBS) -o $@

$(BUILD)/%.o: %.cc | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD) $(OUT)

-include $(DEPS)

.PHONY: all clean
