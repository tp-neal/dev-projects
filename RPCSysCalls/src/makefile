CC := gcc
CFLAGS := -Wall -Wextra -pedantic

EXECS := server user
OBJ_FILES := client.o util.o

TEXT_FILES := local_copy.md remote_copy.md
TEXT_FILE_DIR := "../text_files"

all: $(EXECS) $(OBJ_FILES)

user: user.c client.o util.o client.h error.h
	$(CC) $(CFLAGS) -o $@ user.c client.o util.o

server: server.c util.o server.h error.h
	$(CC) $(CFLAGS) -o $@ server.c util.o

client.o: client.c util.o client.h error.h
	$(CC) $(CFLAGS) -c client.c

util.o: util.c util.h
	$(CC) $(CFLAGS) -c util.c

clean:
	rm -f $(EXECS) $(OBJ_FILES)
	cd $(TEXT_FILE_DIR) && rm -f $(TEXT_FILES)

.PHONY:
	all clean buildprint echo
