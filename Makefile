CC = cc
FLAGS = -O3 -Wall -lgmp
LINK = -lgmp -s

SRC = src
BIN = bin

UTILS = $(SRC)/get_ns.c $(SRC)/print_calc_time.c
PROGS = $(SRC)/fib.c $(SRC)/lucas.c

BINS = $(BIN)/fib $(BIN)/lucas

all: $(BINS)

$(BIN)/%: $(SRC)/%.c $(UTILS)
	mkdir -p $(dir $@)
	$(CC) $(FLAGS) $< $(UTILS) -o $@ $(LINK)

clean:
	rm -rf $(BIN)
