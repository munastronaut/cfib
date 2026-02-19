CC = cc
FLAGS = -O3 -Wall -lgmp
LINK = -lgmp -s

SRC = src
BIN = bin

UTILS = $(SRC)/common.c
TGTS = fib lucas

all: $(addprefix $(BIN)/, $(TGTS))

$(BIN)/%: $(SRC)/%.c $(UTILS)
	mkdir -p $(BIN)
	$(CC) $(FLAGS) $^ -o $@ $(LINK)

clean:
	rm -rf $(BIN)
