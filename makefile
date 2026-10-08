CC = gcc
CFLAGS = -std=gnu11 -Wall -Wextra -Wno-unused-parameter -Wno-format-truncation
SRC = $(wildcard *.c)
HDR = $(wildcard *.h)

a.out: $(SRC) $(HDR)
	$(CC) $(CFLAGS) -o a.out $(SRC)

clean:
	rm -f a.out

.PHONY: clean
