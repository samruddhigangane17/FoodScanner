CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -g -Isrc
SRC     = $(wildcard src/*.c)
LIBSRC  = $(filter-out src/main.c,$(SRC))

scanner: $(SRC) src/common.h
	$(CC) $(CFLAGS) -o scanner $(SRC)

test: tests/test_core.c $(LIBSRC) src/common.h
	$(CC) $(CFLAGS) -o test_core tests/test_core.c $(LIBSRC)
	./test_core

clean:
	rm -f scanner scanner.exe test_core test_core.exe ocr_out.txt

.PHONY: test clean
