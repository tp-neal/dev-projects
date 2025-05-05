
CFLAGS = -Wall
CC = gcc

COMPILED_FILES = compress decompress bit_ops.o
TEST_FILES_DIRECTORY="../canterbury"

all: compress decompress bit_ops.o

compress: compress.c bit_ops.o
	$(CC) $(CFLAGS) -o $@ $^

decompress: decompress.c bit_ops.o
	$(CC) $(CFLAGS) -o $@ $^

bit_ops.o: bit_ops.c bit_ops.h
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(COMPILED_FILES)
	rm -f "$(TEST_FILES_DIRECTORY)"/*.czy

.PHONY: all clean