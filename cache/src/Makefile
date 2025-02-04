# Compiler settings
CC = gcc
FLAGS = -Wall -Wextra

# Project files
OBJ_FILES = main.o
EXE_FILE = cache_exec
HEADER_FILES = cache.h error.h

# Phony targets
.PHONY: all clean

# Default target
all: $(EXE_FILE)

# Linking step
$(EXE_FILE): $(OBJ_FILES)
	$(CC) $(FLAGS) -o $@ $^ -lm

# Compilation step
main.o: main.c $(HEADER_FILES)
	$(CC) $(FLAGS) -c $< -o $@ -MMD

# Include dependencies
-include $(OBJ_FILES:.o=.d)

# Cleanup
clean:
	rm -f $(OBJ_FILES) $(EXE_FILE) *.d``