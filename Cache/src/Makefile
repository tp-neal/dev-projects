# Compiler settings
CC = gcc
FLAGS = -Wall -Wextra

# Project files
OBJ_FILES = main.o
EXE_FILE = cache_exec
HEADER_FILES = cache.h error.h

# Default target
all: $(EXE_FILE)

$(EXE_FILE): $(OBJ_FILES)
	$(CC) $(FLAGS) -o $@ $^ -lm

main.o: main.c $(HEADER_FILES)
	$(CC) $(FLAGS) -o $@ -c $<


.PHONY: all clean

clean:
	rm -f $(OBJ_FILES) $(EXE_FILE)