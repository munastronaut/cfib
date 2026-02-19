CC = cc
FLAGS = -O3 -Wall
LINK = -lgmp -s

SRC = src
BIN = bin

COMMON = $(SRC)/common.c
TGTS = fib lucas

all: $(addprefix $(BIN)/, $(TGTS))

$(BIN)/%: $(SRC)/%.c $(COMMON)
	mkdir -p $(BIN)
	$(CC) $(FLAGS) $^ -o $@ $(LINK)

clean:
	rm -rf $(BIN)
