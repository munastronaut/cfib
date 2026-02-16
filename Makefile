CC = cc
FLAGS = -O3 -Wall -lgmp

SRC = src
BIN = bin

SRCS = $(wildcard $(SRC)/*.c)

BINS = $(patsubst $(SRC)/%.c, $(BIN)/%, $(SRCS))

all: $(BINS)

$(BIN)/%: $(SRC)/%.c
	$(CC) $(FLAGS) $< -o $@

clean:
	rm -rf $(BIN)
