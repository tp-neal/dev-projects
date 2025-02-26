# Compiler settings
CC = gcc
FLAGS = -Wall -Wextra

# Project files
EXE_FILE = cache_exec
OBJ_FILES = cache.o
HEADER_FILES = cache.h config.h error.h

# Default target
all: $(EXE_FILE)

$(EXE_FILE): $(OBJ_FILES)
	$(CC) $(FLAGS) -o $@ $< -lm

cache.o: cache.c $(HEADER_FILES)
	$(CC) $(FLAGS) -c $<

clean:
	rm -f $(EXE_FILE) $(OBJ_FILES)

.PHONY: all clean