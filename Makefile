CC = gcc
FLAGS = -O3 -Wall -lgmp

SRC = src/fib.c
TGT = bin/fib

all: $(TGT)

$(TGT): $(SRC)
	$(CC) $(FLAGS) -o $(TGT) $(SRC)

clean:
	rm -f $(TGT)
